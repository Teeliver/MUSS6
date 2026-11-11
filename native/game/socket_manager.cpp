#include "../../GameServer/stdafx.h"
#include "socket_manager.hpp"
#include "../../GameServer/BlackList.h"
#include "../../GameServer/HackCheck.h"
#include "../../GameServer/IpManager.h"
#include "../../GameServer/Log.h"
#include "../../GameServer/Map.h"
#include <cstdlib>
#include "../../GameServer/PacketManager.h"
#include "../../GameServer/Protocol.h"
#include "../../GameServer/Queue.h"
#include "../../GameServer/SerialCheck.h"
#include "../../GameServer/ServerInfo.h"
#include "../../GameServer/User.h"

#include <array>
#include <cerrno>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <mutex>
#include <poll.h>
#include <sys/socket.h>
#include <unordered_map>
#include <unistd.h>
#include <netinet/tcp.h>

#if !defined(MU_NATIVE_MACOS) || GAMESERVER_UPDATE != 603
#error "The native client socket adapter currently targets EX603"
#endif

namespace
{
	bool prepareSocket(int descriptor)
	{
		const int flags = ::fcntl(descriptor,F_GETFL,0);
		if(flags < 0 || ::fcntl(descriptor,F_SETFL,flags|O_NONBLOCK) < 0 ||
			::fcntl(descriptor,F_SETFD,FD_CLOEXEC) < 0) return false;
#if defined(SO_NOSIGPIPE)
		const int noSignal = 1;
		if(::setsockopt(descriptor,SOL_SOCKET,SO_NOSIGPIPE,&noSignal,sizeof(noSignal)) != 0) return false;
#endif
		const int noDelay = 1;
		if(::setsockopt(descriptor,IPPROTO_TCP,TCP_NODELAY,&noDelay,sizeof(noDelay)) != 0) return false;
		return true;
	}
	int sendFlags()
	{
#if defined(MSG_NOSIGNAL)
		return MSG_NOSIGNAL;
#else
		return 0;
#endif
	}
	bool unsupported(const char* operation)
	{
		errno = ENOTSUP;
		gLog.Output(LOG_CONNECT,"[NativeSocketManager] %s is unavailable; use NativePoll",operation);
		return false;
	}
}

struct CSocketManager::NativeState
{
	struct Peer { int socket; std::uint64_t generation; };
	struct Packet { QUEUE_INFO info{}; std::uint64_t generation; };
	std::recursive_mutex mutex;
	int listener = -1;
	WORD port = 0;
	std::string bindAddress = "0.0.0.0";
	std::string error;
	std::uint64_t nextGeneration = 0;
	std::unordered_map<int,Peer> peers;
	std::deque<Packet> queue;

	~NativeState()
	{
		// Static destruction must not call other, already destroyed gameplay globals.
		if(listener >= 0) ::close(listener);
		for(const auto& pair : peers)
		{
			// Normal runtime Clean retires all entries before static destruction.
			::close(pair.second.socket);
		}
	}

	bool fail(const char* operation,int code)
	{
		error = std::string(operation) + ": " + std::strerror(code);
		gLog.Output(LOG_CONNECT,"[NativeSocketManager] %s",error.c_str());
		return false;
	}
	bool live(int index,const Peer& peer)
	{
		return OBJECT_USER_RANGE(index) != 0 && gObj[index].Connected != OBJECT_OFFLINE &&
			gObj[index].Socket == peer.socket && gObj[index].PerSocketContext != nullptr;
	}
	void retireDetached()
	{
		for(auto it = peers.begin(); it != peers.end();)
		{
			if(live(it->first,it->second)) { ++it; continue; }
			// Offline attack/store/helper commands close the context fd directly and
			// keep the avatar online. Never close that numeric fd again or delete it.
			if(OBJECT_USER_RANGE(it->first) && gObj[it->first].Connected != OBJECT_OFFLINE &&
				gObj[it->first].Socket == INVALID_SOCKET && gObj[it->first].PerSocketContext != nullptr)
				gObj[it->first].PerSocketContext->Socket = INVALID_SOCKET;
			it = peers.erase(it);
		}
	}
};

CSocketManager gSocketManager;

CSocketManager::CSocketManager() : m_native(std::make_unique<NativeState>()) {}
CSocketManager::~CSocketManager() = default;

void CSocketManager::SetBindAddress(const char* address)
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	if(m_native->listener >= 0) { m_native->error = "SetBindAddress requires a stopped listener"; return; }
	m_native->bindAddress = address == nullptr ? "0.0.0.0" : address;
}

bool CSocketManager::Start(WORD port)
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	if(m_native->listener >= 0) return false;
	m_native->port = port;
	if(!CreateListenSocket()) return false;
	gLog.Output(LOG_CONNECT,"[NativeSocketManager] Server started at port [%u]",static_cast<unsigned>(m_native->port));
	return true;
}

bool CSocketManager::CreateListenSocket()
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.listener >= 0) return false;
	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_port = htons(state.port);
	if(::inet_pton(AF_INET,state.bindAddress.c_str(),&address.sin_addr) != 1)
		return state.fail("Invalid IPv4 bind address",EINVAL);
	const int descriptor = ::socket(AF_INET,SOCK_STREAM,0);
	if(descriptor < 0) return state.fail("socket",errno);
	const int reuse = 1;
	if(::setsockopt(descriptor,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse)) != 0 ||
		!prepareSocket(descriptor) || ::bind(descriptor,reinterpret_cast<sockaddr*>(&address),sizeof(address)) != 0 ||
		::listen(descriptor,5) != 0)
	{
		const int code = errno;
		::close(descriptor);
		return state.fail("CreateListenSocket",code);
	}
	socklen_t length = sizeof(address);
	if(::getsockname(descriptor,reinterpret_cast<sockaddr*>(&address),&length) != 0)
	{
		const int code = errno;
		::close(descriptor);
		return state.fail("getsockname",code);
	}
	state.listener = descriptor;
	state.port = ntohs(address.sin_port);
	state.error.clear();
	return true;
}

void CSocketManager::Clean()
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	state.retireDetached();
	while(!state.peers.empty()) Disconnect(state.peers.begin()->first);
	state.queue.clear();
	if(state.listener >= 0) { ::close(state.listener); state.listener = -1; }
}

bool CSocketManager::CreateCompletionPort() { return unsupported("CreateCompletionPort"); }
bool CSocketManager::CreateAcceptThread() { return unsupported("CreateAcceptThread"); }
bool CSocketManager::CreateWorkerThread() { return unsupported("CreateWorkerThread"); }
bool CSocketManager::CreateServerQueue() { return unsupported("CreateServerQueue"); }
DWORD CSocketManager::ServerAcceptThread(CSocketManager*) { unsupported("ServerAcceptThread"); return ENOTSUP; }
DWORD CSocketManager::ServerWorkerThread(CSocketManager*) { unsupported("ServerWorkerThread"); return ENOTSUP; }
DWORD CSocketManager::ServerQueueThread(CSocketManager*) { unsupported("ServerQueueThread"); return ENOTSUP; }

int CSocketManager::ServerAcceptCondition(LPWSABUF callerId,LPWSABUF,LPQOS,LPQOS,LPWSABUF,LPWSABUF,GROUP*,CSocketManager*)
{
	if(callerId == nullptr || callerId->buf == nullptr || callerId->len < sizeof(sockaddr_in)) return 1;
	const auto* address = reinterpret_cast<const sockaddr_in*>(callerId->buf);
	char ip[INET_ADDRSTRLEN]{};
	if(::inet_ntop(AF_INET,&address->sin_addr,ip,sizeof(ip)) == nullptr) return 1;
	return gBlackList.CheckIpAddress(ip) && gIpManager.CheckIpAddress(ip) ? 0 : 1;
}

bool CSocketManager::DataRecv(int index,IO_MAIN_BUFFER* buffer)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	auto peer = state.peers.find(index);
	if(peer == state.peers.end() || !state.live(index,peer->second) || buffer == nullptr ||
		buffer->size < 0 || buffer->size > MAX_MAIN_PACKET_SIZE) return false;
	while(buffer->size > 0)
	{
		const BYTE type = buffer->buff[0];
		if(type != 0xC1 && type != 0xC2 && type != 0xC3 && type != 0xC4) return false;
		const bool wide = type == 0xC2 || type == 0xC4;
		const int headerSize = wide ? 4 : 3;
		if(buffer->size < headerSize) return true;
		const int size = wide ? (static_cast<int>(buffer->buff[1])<<8)|buffer->buff[2] : buffer->buff[1];
		if(size < headerSize || size > MAX_MAIN_PACKET_SIZE) return false;
		if(buffer->size < size) return true;
		auto& object = gObj[index];
		if(gServerInfo.m_MaxPacketPerSecond != 0 && (++object.PacketCount > gServerInfo.m_MaxPacketPerSecond))
		{
			gLog.Output(LOG_HACK,"[%s] Client disconnected, packets exceeded. [%u][%u]",object.IpAddr,
				static_cast<unsigned>(object.PacketCount),static_cast<unsigned>(gServerInfo.m_MaxPacketPerSecond));
			return false;
		}
		NativeState::Packet packet{};
		packet.generation = peer->second.generation;
		packet.info.index = static_cast<WORD>(index);
		std::array<BYTE,MAX_MAIN_PACKET_SIZE> normalized{};
		int normalizedSize = size;
		if(type == 0xC3 || type == 0xC4)
		{
			const int cipherOffset = wide ? 3 : 2;
			const int cipherSize = size-cipherOffset;
			if(cipherSize <= 0 || (cipherSize%11) != 0) return false;
			const int decodeOffset = wide ? 2 : 1;
			int plainSize = 0;
			for(int offset = 0; offset < cipherSize; offset += 11)
			{
				// Original Decrypt's stale-result check misses a failed later block.
				// Calling the original block primitive preserves valid ciphertext and
				// checks every checksum/length before framing or protocol dispatch.
				const int blockSize = gPacketManager.DecryptBlock(normalized.data()+decodeOffset+plainSize,buffer->buff+cipherOffset+offset);
				if(blockSize < 1 || blockSize > 8 || (offset+11 < cipherSize && blockSize != 8)) return false;
				plainSize += blockSize;
			}
			if(plainSize < 2) return false; // Serial followed by packet head.
			packet.info.serial = normalized[decodeOffset];
			packet.info.encrypt = 1;
			normalizedSize = plainSize+decodeOffset;
			if(!wide && normalizedSize > 255) return false;
			normalized[0] = wide ? 0xC2 : 0xC1;
			if(wide) { normalized[1] = HIBYTE(normalizedSize); normalized[2] = LOBYTE(normalizedSize); }
			else normalized[1] = static_cast<BYTE>(normalizedSize);
		}
		else
		{
			std::memcpy(normalized.data(),buffer->buff,size);
			packet.info.encrypt = 0;
			packet.info.serial = static_cast<DWORD>(-1);
		}
		packet.info.head = normalized[headerSize-1];
		packet.info.size = static_cast<DWORD>(normalizedSize);
		if(!gPacketManager.AddData(normalized.data(),normalizedSize) || !gPacketManager.ExtractPacket(packet.info.buff)) return false;
		if(state.queue.size() < MAX_QUEUE_SIZE) state.queue.push_back(std::move(packet));
		else
		{
			// Match the original bounded CQueue behavior: complete overflow packets
			// are dropped. NativeLastError makes this loss visible to diagnostics.
			state.error = "Protocol queue limit reached; packet dropped";
		}
		buffer->size -= size;
		if(buffer->size > 0) std::memmove(buffer->buff,buffer->buff+size,buffer->size);
	}
	return true;
}

bool CSocketManager::DataSend(int index,BYTE* message,int size)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	auto peer = state.peers.find(index);
	if(peer == state.peers.end() || !state.live(index,peer->second)) return false;
	if(message == nullptr || size < 3 || size > MAX_MAIN_PACKET_SIZE) { Disconnect(index); return false; }
	const BYTE type = message[0];
	const bool wide = type == 0xC2 || type == 0xC4;
	const int headerSize = wide ? 4 : 3;
	if((type != 0xC1 && type != 0xC2 && type != 0xC3 && type != 0xC4) || size < headerSize ||
		(wide ? ((static_cast<int>(message[1])<<8)|message[2]) : message[1]) != size)
	{
		Disconnect(index);
		return false;
	}
	std::array<BYTE,MAX_MAIN_PACKET_SIZE> output{};
	int outputSize = size;
	if(type == 0xC3 || type == 0xC4)
	{
		const int serialOffset = wide ? 2 : 1;
		const int cipherOffset = serialOffset+1;
		const int cipherSize = gPacketManager.Encrypt(nullptr,message+serialOffset,size-serialOffset);
		if(cipherSize <= 0 || cipherSize+cipherOffset > MAX_MAIN_PACKET_SIZE || (!wide && cipherSize+cipherOffset > 255))
		{
			Disconnect(index);
			return false;
		}
		const int plainSize = size-serialOffset;
		// EncryptBlock reads all eight source bytes even for the final short block.
		// Pad owned storage explicitly rather than reading beyond the caller packet.
		std::vector<BYTE> plaintext(static_cast<std::size_t>((plainSize+7)/8)*8,0);
		std::memcpy(plaintext.data(),message+serialOffset,plainSize);
		plaintext[0] = gSerialCheck[index].GetSendSerial();
		const int encrypted = gPacketManager.Encrypt(output.data()+cipherOffset,plaintext.data(),plainSize);
		if(encrypted != cipherSize) { Disconnect(index); return false; }
		outputSize = encrypted+cipherOffset;
		output[0] = type;
		if(wide) { output[1] = HIBYTE(outputSize); output[2] = LOBYTE(outputSize); }
		else output[1] = static_cast<BYTE>(outputSize);
	}
	else std::memcpy(output.data(),message,size);
#if ENCRYPT_STATE == 1
	EncryptData(output.data(),outputSize);
#endif
	IO_SEND_CONTEXT& context = gObj[index].PerSocketContext->IoSendContext;
	if(context.IoSize > 0)
	{
		if(context.IoSideBuffer.size+outputSize > MAX_SIDE_PACKET_SIZE) { Disconnect(index); return false; }
		std::memcpy(context.IoSideBuffer.buff+context.IoSideBuffer.size,output.data(),outputSize);
		context.IoSideBuffer.size += outputSize;
		return true;
	}
	std::memcpy(context.IoMainBuffer.buff,output.data(),outputSize);
	context.IoMainBuffer.size = 0;
	context.IoSize = outputSize;
	context.IoType = IO_SEND;
	OnSend(index,0,&context); // Flush directly; EAGAIN retains the bounded buffers.
	return state.peers.find(index) != state.peers.end() && gObj[index].Socket != INVALID_SOCKET;
}

void CSocketManager::Disconnect(int index)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	auto peer = state.peers.find(index);
	if(peer == state.peers.end()) return;
	const auto value = peer->second;
	state.peers.erase(peer); // Retire before gObjDel callbacks can reenter transport.
	if(!state.live(index,value)) return;
	::close(value.socket);
	gObj[index].Socket = INVALID_SOCKET;
	gObj[index].PerSocketContext->Socket = INVALID_SOCKET;
	gObjDel(index);
}

void CSocketManager::OnRecv(int index,DWORD size,IO_RECV_CONTEXT* context)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	auto peer = state.peers.find(index);
	if(peer == state.peers.end() || !state.live(index,peer->second) || context == nullptr) return;
	if(size == 0 || context->IoMainBuffer.size < 0 || context->IoMainBuffer.size > MAX_MAIN_PACKET_SIZE ||
		size > static_cast<DWORD>(MAX_MAIN_PACKET_SIZE-context->IoMainBuffer.size))
	{
		Disconnect(index);
		return;
	}
#if ENCRYPT_STATE == 1
	DecryptData(context->IoMainBuffer.buff+context->IoMainBuffer.size,static_cast<int>(size));
#endif
	context->IoMainBuffer.size += static_cast<int>(size);
	if(!DataRecv(index,&context->IoMainBuffer))
	{
		gLog.Output(LOG_HACK,"[NativeDataRecv] Invalid client framing (Index: %d)",index);
		Disconnect(index);
	}
}

void CSocketManager::OnSend(int index,DWORD completed,IO_SEND_CONTEXT* context)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	auto peer = state.peers.find(index);
	if(peer == state.peers.end() || !state.live(index,peer->second) || context == nullptr) return;
	if(context->IoSize < 0 || context->IoSize > MAX_MAIN_PACKET_SIZE || context->IoMainBuffer.size < 0 ||
		context->IoMainBuffer.size > context->IoSize || context->IoSideBuffer.size < 0 ||
		context->IoSideBuffer.size > MAX_SIDE_PACKET_SIZE) { Disconnect(index); return; }
	if(completed > static_cast<DWORD>(context->IoSize-context->IoMainBuffer.size)) { Disconnect(index); return; }
	context->IoMainBuffer.size += static_cast<int>(completed);
	for(;;)
	{
		if(context->IoMainBuffer.size >= context->IoSize)
		{
			if(context->IoSideBuffer.size <= 0) { context->IoSize = 0; context->IoMainBuffer.size = 0; return; }
			const int nextSize = std::min(context->IoSideBuffer.size,MAX_MAIN_PACKET_SIZE);
			std::memcpy(context->IoMainBuffer.buff,context->IoSideBuffer.buff,nextSize);
			context->IoSideBuffer.size -= nextSize;
			if(context->IoSideBuffer.size > 0) std::memmove(context->IoSideBuffer.buff,context->IoSideBuffer.buff+nextSize,context->IoSideBuffer.size);
			context->IoMainBuffer.size = 0;
			context->IoSize = nextSize;
		}
		const auto result = ::send(peer->second.socket,context->IoMainBuffer.buff+context->IoMainBuffer.size,
			static_cast<std::size_t>(context->IoSize-context->IoMainBuffer.size),sendFlags());
		if(result > 0) { context->IoMainBuffer.size += static_cast<int>(result); continue; }
		if(result < 0 && errno == EINTR) continue;
		if(result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
		state.fail("send",result == 0 ? EPIPE : errno);
		Disconnect(index);
		return;
	}
}

bool CSocketManager::NativePoll(int timeoutMs)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.listener < 0) return false;
	state.retireDetached();
	struct Slot { int index; std::uint64_t generation; };
	std::vector<pollfd> descriptors;
	std::vector<Slot> slots;
	descriptors.push_back({state.listener,POLLIN,0});
	slots.push_back({-1,0});
	for(const auto& pair : state.peers)
	{
		const auto& send = gObj[pair.first].PerSocketContext->IoSendContext;
		descriptors.push_back({pair.second.socket,static_cast<short>(POLLIN | (send.IoSize > 0 ? POLLOUT : 0)),0});
		slots.push_back({pair.first,pair.second.generation});
	}
	int result;
	do { result = ::poll(descriptors.data(),descriptors.size(),std::clamp(timeoutMs,0,60000)); } while(result < 0 && errno == EINTR);
	if(result < 0) return state.fail("poll",errno);
	if(descriptors[0].revents & (POLLERR|POLLHUP|POLLNVAL)) return state.fail("Listener poll",EIO);
	if(descriptors[0].revents & POLLIN)
	{
		// Bound accepts per tick so timer callbacks continue under connection load.
		for(int count = 0; count < MAX_OBJECT_USER; ++count)
		{
			sockaddr_in address{};
			socklen_t length = sizeof(address);
			const int socket = ::accept(state.listener,reinterpret_cast<sockaddr*>(&address),&length);
			if(socket < 0)
			{
				if(errno == EINTR) { --count; continue; }
				if(errno == EAGAIN || errno == EWOULDBLOCK) break;
				state.fail("accept",errno);
				break;
			}
			char ip[INET_ADDRSTRLEN]{};
			if(::inet_ntop(AF_INET,&address.sin_addr,ip,sizeof(ip)) == nullptr ||
				!gBlackList.CheckIpAddress(ip) || !gIpManager.CheckIpAddress(ip)) { ::close(socket); continue; }
			// gObjAddSearch sends the original capacity/closing result itself.
			const int index = gObjAddSearch(socket,ip);
			if(index < 0 || !prepareSocket(socket)) { ::close(socket); continue; }
			if(gObjAdd(socket,ip,index) < 0) { ::close(socket); continue; }
			auto* context = gObj[index].PerSocketContext;
			if(context == nullptr) { ::close(socket); gObj[index].Socket = INVALID_SOCKET; gObjDel(index); continue; }
			std::memset(context,0,sizeof(*context));
			context->Socket = socket;
			context->Index = index;
			context->IoRecvContext.IoType = IO_RECV;
			context->IoRecvContext.wsabuf.buf = reinterpret_cast<char*>(context->IoRecvContext.IoMainBuffer.buff);
			context->IoRecvContext.wsabuf.len = MAX_MAIN_PACKET_SIZE;
			context->IoSendContext.IoType = IO_SEND;
			context->IoSendContext.wsabuf.buf = reinterpret_cast<char*>(context->IoSendContext.IoMainBuffer.buff);
			state.peers[index] = {socket,++state.nextGeneration};
			GCConnectClientSend(index,1);
		}
	}
	for(std::size_t position = 1; position < descriptors.size(); ++position)
	{
		const int index = slots[position].index;
		auto peer = state.peers.find(index);
		if(peer == state.peers.end() || peer->second.generation != slots[position].generation || !state.live(index,peer->second)) continue;
		if(descriptors[position].revents & POLLNVAL) { Disconnect(index); continue; }
		if(descriptors[position].revents & (POLLIN|POLLHUP|POLLERR))
		{
			auto* context = &gObj[index].PerSocketContext->IoRecvContext;
			const int capacity = MAX_MAIN_PACKET_SIZE-context->IoMainBuffer.size;
			if(capacity <= 0) { Disconnect(index); continue; }
			ssize_t received;
			do { received = ::recv(peer->second.socket,context->IoMainBuffer.buff+context->IoMainBuffer.size,capacity,0); } while(received < 0 && errno == EINTR);
			if(received > 0) OnRecv(index,static_cast<DWORD>(received),context);
			else if(received == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) { Disconnect(index); continue; }
		}
		peer = state.peers.find(index);
		if(peer != state.peers.end() && peer->second.generation == slots[position].generation && state.live(index,peer->second) &&
			(descriptors[position].revents & POLLOUT)) OnSend(index,0,&gObj[index].PerSocketContext->IoSendContext);
	}
	while(!state.queue.empty())
	{
		auto packet = std::move(state.queue.front());
		state.queue.pop_front();
		const int index = packet.info.index;
		auto peer = state.peers.find(index);
		if(peer == state.peers.end() || peer->second.generation != packet.generation || !state.live(index,peer->second)) continue;
		// Opt-in packet diagnostics; no account credentials or gameplay changes.
        static const bool traceCombat = std::getenv("MU_NATIVE_COMBAT_TRACE") != nullptr;
        int combatTarget = -1;
        if(traceCombat && ((packet.info.head == PROTOCOL_CODE2 && packet.info.size >= 7) || (packet.info.head == 0x19 && packet.info.size >= 8))) {
            const unsigned offset = packet.info.head == PROTOCOL_CODE2 ? 3 : 5;
            combatTarget = (unsigned(packet.info.buff[offset]) << 8) | packet.info.buff[offset+1];
            if(OBJECT_RANGE(combatTarget)) {
                auto& attacker = gObj[index]; auto& target = gObj[combatTarget];
                std::fprintf(stderr,"[CombatTrace] head=%02X self=%d connected=%d map=%d xy=%d,%d teleport=%d safe=%d target=%d type=%d map=%d xy=%d,%d live=%d state=%d hp=%.0f safe=%d\n",packet.info.head,index,attacker.Connected,attacker.Map,attacker.X,attacker.Y,attacker.Teleport,gMap[attacker.Map].CheckAttr(attacker.X,attacker.Y,1),combatTarget,target.Type,target.Map,target.X,target.Y,target.Live,target.State,target.Life,gMap[target.Map].CheckAttr(target.X,target.Y,1));
            }
        }
		ProtocolCore(packet.info.head,packet.info.buff,packet.info.size,index,packet.info.encrypt,packet.info.serial);
        if(traceCombat && OBJECT_RANGE(combatTarget)) std::fprintf(stderr,"[CombatTrace] after target=%d hp=%.0f live=%d\n",combatTarget,gObj[combatTarget].Life,gObj[combatTarget].Live);
		state.retireDetached();
	}
	return true;
}

DWORD CSocketManager::GetQueueSize()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return static_cast<DWORD>(m_native->queue.size());
}

WORD CSocketManager::NativePort()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return m_native->port;
}

std::string CSocketManager::NativeLastError()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return m_native->error;
}
