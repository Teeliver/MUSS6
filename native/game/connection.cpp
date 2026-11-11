#include "../../GameServer/Connection.h"

#if !defined(MU_NATIVE_MACOS)
#error "The POSIX connection adapter requires MU_NATIVE_MACOS"
#endif

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

struct CConnection::NativeState
{
	std::recursive_mutex mutex;
	int socket = -1;
	ProtocolCallback protocol = nullptr;
	std::vector<BYTE> receive;
	std::vector<BYTE> send;
	std::array<BYTE,MAX_BUFF_SIZE> dispatch{};
	std::size_t sendOffset = 0;
	std::uint64_t generation = 0;
	bool closePending = false;
	int connectTimeoutMs = 5000;
	std::string error;

	void close(bool report)
	{
		if(socket >= 0)
		{
			::close(socket);
			socket = -1;
			++generation;
			if(report) closePending = true;
		}
		receive.clear();
		send.clear();
		sendOffset = 0;
	}

	bool fail(const char* operation,int code)
	{
		error = std::string(operation) + ": " + std::strerror(code);
		close(true);
		return false;
	}

	bool invalid(const char* reason)
	{
		error = reason;
		close(true);
		return false;
	}

	bool flush()
	{
		if(socket < 0) return false;
		while(sendOffset < send.size())
		{
			int flags = 0;
#if defined(MSG_NOSIGNAL)
			flags = MSG_NOSIGNAL;
#endif
			const auto result = ::send(socket,send.data()+sendOffset,send.size()-sendOffset,flags);
			if(result > 0)
			{
				sendOffset += static_cast<std::size_t>(result);
				continue;
			}
			if(result < 0 && errno == EINTR) continue;
			if(result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return true;
			return fail("send",result == 0 ? EPIPE : errno);
		}
		send.clear();
		sendOffset = 0;
		return true;
	}
};

CConnection::CConnection() : m_native(std::make_unique<NativeState>()) {}
CConnection::~CConnection() { Disconnect(); }

void CConnection::Init(HWND hwnd,void* function)
{
	Init(hwnd,reinterpret_cast<ProtocolCallback>(function));
}

void CConnection::Init(HWND,ProtocolCallback function)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	state.close(false);
	state.closePending = false;
	state.error.clear();
	state.protocol = function;
}

bool CConnection::Connect(char* address,WORD port,DWORD)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.socket >= 0) return true;
	if(address == nullptr || address[0] == '\0' || port == 0 || state.protocol == nullptr)
		return state.invalid("Connect requires an address, port and initialized protocol callback");

	addrinfo hints{};
	hints.ai_family = AF_INET; // Original service endpoints and packet addresses are IPv4.
	hints.ai_socktype = SOCK_STREAM;
	addrinfo* resolved = nullptr;
	const std::string service = std::to_string(port);
	const int resolveResult = ::getaddrinfo(address,service.c_str(),&hints,&resolved);
	if(resolveResult != 0)
	{
		state.error = std::string("getaddrinfo: ") + ::gai_strerror(resolveResult);
		return false;
	}
	std::unique_ptr<addrinfo,decltype(&::freeaddrinfo)> addresses(resolved,&::freeaddrinfo);
	state.error.clear();
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(state.connectTimeoutMs);
	for(auto* target = addresses.get(); target != nullptr; target = target->ai_next)
	{
		const int descriptor = ::socket(target->ai_family,target->ai_socktype,target->ai_protocol);
		if(descriptor < 0) { state.error = std::string("socket: ") + std::strerror(errno); continue; }
		const auto closeAttempt = [&]() { ::close(descriptor); };
		const int currentFlags = ::fcntl(descriptor,F_GETFL,0);
		if(currentFlags < 0 || ::fcntl(descriptor,F_SETFL,currentFlags|O_NONBLOCK) < 0 ||
			::fcntl(descriptor,F_SETFD,FD_CLOEXEC) < 0)
		{
			state.error = std::string("fcntl: ") + std::strerror(errno);
			closeAttempt();
			continue;
		}
#if defined(SO_NOSIGPIPE)
		const int noSignal = 1;
		if(::setsockopt(descriptor,SOL_SOCKET,SO_NOSIGPIPE,&noSignal,sizeof(noSignal)) != 0)
		{
			state.error = std::string("SO_NOSIGPIPE: ") + std::strerror(errno);
			closeAttempt();
			continue;
		}
#endif
		int connectError = 0;
		if(::connect(descriptor,target->ai_addr,target->ai_addrlen) != 0)
		{
			connectError = errno;
			if(connectError == EINPROGRESS || connectError == EINTR)
			{
				for(;;)
				{
					const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline-std::chrono::steady_clock::now()).count();
					if(remaining <= 0) { connectError = ETIMEDOUT; break; }
					pollfd ready{descriptor,POLLOUT,0};
					const int result = ::poll(&ready,1,static_cast<int>(remaining));
					if(result < 0 && errno == EINTR) continue;
					if(result < 0) { connectError = errno; break; }
					if(result == 0) { connectError = ETIMEDOUT; break; }
					socklen_t length = sizeof(connectError);
					if(::getsockopt(descriptor,SOL_SOCKET,SO_ERROR,&connectError,&length) != 0) connectError = errno;
					break;
				}
			}
		}
		if(connectError != 0)
		{
			state.error = std::string("connect: ") + std::strerror(connectError);
			closeAttempt();
			continue;
		}
		state.socket = descriptor;
		++state.generation;
		state.receive.clear();
		state.send.clear();
		state.sendOffset = 0;
		state.closePending = false;
		state.error.clear();
		return true;
	}
	return false;
}

void CConnection::Disconnect()
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	state.close(false);
}

bool CConnection::CheckState()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return m_native->socket >= 0;
}

bool CConnection::DataRecv()
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.socket < 0) return false;
	std::array<BYTE,16384> input{};
	ssize_t received;
	do { received = ::recv(state.socket,input.data(),input.size(),0); } while(received < 0 && errno == EINTR);
	if(received < 0)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK) return true;
		return state.fail("recv",errno);
	}
	if(received == 0) return state.invalid("Service connection closed by peer");
	if(state.receive.size()+static_cast<std::size_t>(received) > MAX_BUFF_SIZE)
		return state.invalid("Service receive buffer limit exceeded");
	state.receive.insert(state.receive.end(),input.begin(),input.begin()+received);
	for(;;)
	{
		if(state.receive.empty()) return true;
		const BYTE type = state.receive[0];
		if(type != 0xC1 && type != 0xC2) return state.invalid("Invalid service packet type (expected C1/C2)");
		const std::size_t headerSize = type == 0xC1 ? 3 : 4;
		if(state.receive.size() < headerSize) return true;
		const std::size_t size = type == 0xC1 ? state.receive[1] :
			(static_cast<std::size_t>(state.receive[1])<<8)|state.receive[2];
		if(size < headerSize || size > MAX_BUFF_SIZE) return state.invalid("Invalid service packet size");
		if(state.receive.size() < size) return true;
		// Consume before callback: callback may send, disconnect or reinitialize.
		// Legacy service handlers cast whole structs without a size argument. Keep
		// the original MAX_BUFF_SIZE backing allocation and a zero tail rather
		// than exposing an exact-size heap allocation to those handlers. The size
		// passed to the callback remains the actual frame size; this is not schema
		// validation of fixed bodies or count-controlled variable records.
		state.dispatch.fill(0);
		std::copy(state.receive.begin(),state.receive.begin()+size,state.dispatch.begin());
		state.receive.erase(state.receive.begin(),state.receive.begin()+size);
		const auto generation = state.generation;
		state.protocol(state.dispatch[headerSize-1],state.dispatch.data(),static_cast<int>(size));
		if(state.socket < 0 || state.generation != generation) return false;
	}
}

bool CConnection::DataSend(BYTE* message,int size,int)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.socket < 0) return false;
	if(size < 0 || (size > 0 && message == nullptr)) return state.invalid("Invalid service send buffer");
	if(size == 0) return true;
	const auto queued = state.send.size()-state.sendOffset;
	if(static_cast<std::size_t>(size) > MAX_BUFF_SIZE-queued) return state.invalid("Service send buffer limit exceeded");
	if(state.sendOffset != 0)
	{
		state.send.erase(state.send.begin(),state.send.begin()+state.sendOffset);
		state.sendOffset = 0;
	}
	state.send.insert(state.send.end(),message,message+size);
	return state.flush();
}

bool CConnection::DataSendEx()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return m_native->flush();
}

unsigned CConnection::NativePollEvents(int timeoutMs)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	unsigned events = state.closePending ? NativeClose : 0;
	if(state.socket < 0) return events;
	pollfd ready{state.socket,static_cast<short>(POLLIN | (state.sendOffset < state.send.size() ? POLLOUT : 0)),0};
	int result;
	do { result = ::poll(&ready,1,std::clamp(timeoutMs,0,60000)); } while(result < 0 && errno == EINTR);
	if(result < 0) { state.fail("poll",errno); return NativeClose; }
	if(ready.revents & POLLNVAL) { state.invalid("Invalid service socket descriptor"); return NativeClose; }
	// Read on HUP/ERR too, so complete buffered frames are delivered before EOF.
	if(ready.revents & (POLLIN|POLLHUP|POLLERR)) events |= NativeRead;
	if(ready.revents & POLLOUT) events |= NativeWrite;
	return events;
}

bool CConnection::NativeTakeCloseEvent()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	const bool pending = m_native->closePending;
	m_native->closePending = false;
	return pending;
}

void CConnection::NativeSetConnectTimeout(int timeoutMs)
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	m_native->connectTimeoutMs = std::clamp(timeoutMs,1,60000);
}

std::string CConnection::NativeLastError()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return m_native->error;
}
std::size_t CConnection::NativePendingSendBytes(){
 std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
 return m_native->send.size()-m_native->sendOffset;
}
