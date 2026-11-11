#include "../../GameServer/stdafx.h"
#include "socket_manager_udp.hpp"
#include "../../GameServer/Log.h"

#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

struct CSocketManagerUdp::NativeState
{
	std::recursive_mutex mutex;
	int socket = -1;
	WORD port = 0;
	std::string bindAddress = "0.0.0.0";
	sockaddr_in destination{};
	bool hasDestination = false;
	std::vector<BYTE> receive;
	std::vector<BYTE> send;
	std::string error;
	~NativeState() { if(socket >= 0) ::close(socket); }
	bool fail(const char* operation,int code)
	{
		error = std::string(operation) + ": " + std::strerror(code);
		gLog.Output(LOG_CONNECT,"[NativeSocketManagerUdp] %s",error.c_str());
		return false;
	}
	bool create()
	{
		const int descriptor = ::socket(AF_INET,SOCK_DGRAM,0);
		if(descriptor < 0) return fail("socket",errno);
		const int flags = ::fcntl(descriptor,F_GETFL,0);
		if(flags < 0 || ::fcntl(descriptor,F_SETFL,flags|O_NONBLOCK) != 0 || ::fcntl(descriptor,F_SETFD,FD_CLOEXEC) != 0)
		{
			const int code = errno;
			::close(descriptor);
			return fail("fcntl",code);
		}
		socket = descriptor;
		return true;
	}
	bool flush()
	{
		if(socket < 0 || !hasDestination) return false;
		if(send.empty()) return true;
		ssize_t result;
		do { result = ::sendto(socket,send.data(),send.size(),0,reinterpret_cast<sockaddr*>(&destination),sizeof(destination)); } while(result < 0 && errno == EINTR);
		if(result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return true;
		if(result < 0)
		{
			const int code = errno;
			send.clear();
			return fail("sendto",code);
		}
		if(static_cast<std::size_t>(result) != send.size())
		{
			send.clear();
			return fail("Incomplete UDP datagram",EIO);
		}
		send.clear();
		return true;
	}
};

CSocketManagerUdp gSocketManagerUdp;

CSocketManagerUdp::CSocketManagerUdp() : m_native(std::make_unique<NativeState>()) {}
CSocketManagerUdp::~CSocketManagerUdp() = default;

void CSocketManagerUdp::SetBindAddress(const char* address)
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	if(m_native->socket >= 0) { m_native->error = "SetBindAddress requires a stopped socket"; return; }
	m_native->bindAddress = address == nullptr ? "0.0.0.0" : address;
}

bool CSocketManagerUdp::Start(WORD port)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	Clean();
	if(!state.create()) return false;
	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	if(::inet_pton(AF_INET,state.bindAddress.c_str(),&address.sin_addr) != 1)
	{
		Clean();
		return state.fail("Invalid IPv4 bind address",EINVAL);
	}
	if(::bind(state.socket,reinterpret_cast<sockaddr*>(&address),sizeof(address)) != 0)
	{
		const int code = errno;
		Clean();
		return state.fail("bind",code);
	}
	socklen_t length = sizeof(address);
	if(::getsockname(state.socket,reinterpret_cast<sockaddr*>(&address),&length) != 0)
	{
		const int code = errno;
		Clean();
		return state.fail("getsockname",code);
	}
	state.port = ntohs(address.sin_port);
	state.error.clear();
	return true;
}

bool CSocketManagerUdp::Connect(char* address,WORD port)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	Clean();
	if(address == nullptr || address[0] == '\0' || port == 0) return state.fail("Invalid UDP destination",EINVAL);
	addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	addrinfo* resolved = nullptr;
	const std::string service = std::to_string(port);
	const int result = ::getaddrinfo(address,service.c_str(),&hints,&resolved);
	if(result != 0)
	{
		state.error = std::string("getaddrinfo: ")+::gai_strerror(result);
		return false;
	}
	std::unique_ptr<addrinfo,decltype(&::freeaddrinfo)> addresses(resolved,&::freeaddrinfo);
	if(addresses == nullptr) return state.fail("Missing IPv4 UDP destination",EINVAL);
	std::memcpy(&state.destination,addresses->ai_addr,sizeof(state.destination));
	if(!state.create()) return false;
	state.hasDestination = true;
	state.error.clear();
	return true;
}

void CSocketManagerUdp::Clean()
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.socket >= 0) { ::close(state.socket); state.socket = -1; }
	state.receive.clear();
	state.send.clear();
	state.hasDestination = false;
	state.port = 0;
}

bool CSocketManagerUdp::DataRecv()
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	std::size_t offset = 0;
	while(offset < state.receive.size())
	{
		const auto remaining = state.receive.size()-offset;
		const BYTE type = state.receive[offset];
		if(type != 0xC1 && type != 0xC2) { state.receive.clear(); return state.fail("Invalid UDP packet type",EPROTO); }
		const std::size_t headerSize = type == 0xC1 ? 3 : 4;
		if(remaining < headerSize) { state.receive.clear(); return state.fail("Truncated UDP header",EPROTO); }
		const std::size_t size = type == 0xC1 ? state.receive[offset+1] :
			(static_cast<std::size_t>(state.receive[offset+1])<<8)|state.receive[offset+2];
		if(size < headerSize || size > remaining) { state.receive.clear(); return state.fail("Invalid UDP packet size",EPROTO); }
		// The original GameServer UDP receiver validates and discards packets; it
		// has no protocol callback. Never combine fragments from separate datagrams.
		offset += size;
	}
	state.receive.clear();
	return true;
}

bool CSocketManagerUdp::DataSend(BYTE* message,int size)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.socket < 0 || !state.hasDestination) return false;
	if(message == nullptr || size <= 0 || static_cast<std::size_t>(size) > MAX_UDP_PACKET_SIZE-state.send.size())
	{
		state.send.clear();
		return state.fail("Invalid or oversized UDP send buffer",EMSGSIZE);
	}
	state.send.insert(state.send.end(),message,message+size);
	return state.flush();
}

bool CSocketManagerUdp::NativePoll(int timeoutMs)
{
	auto& state = *m_native;
	std::lock_guard<std::recursive_mutex> lock(state.mutex);
	if(state.socket < 0) return false;
	pollfd descriptor{state.socket,static_cast<short>(POLLIN | (!state.send.empty() ? POLLOUT : 0)),0};
	int result;
	do { result = ::poll(&descriptor,1,std::clamp(timeoutMs,0,60000)); } while(result < 0 && errno == EINTR);
	if(result < 0) return state.fail("poll",errno);
	if(descriptor.revents & (POLLNVAL|POLLHUP|POLLERR)) return state.fail("UDP socket poll",EIO);
	if(descriptor.revents & POLLIN)
	{
		for(int count = 0; count < 64; ++count)
		{
			std::array<BYTE,MAX_UDP_PACKET_SIZE+1> input{};
			const auto received = ::recvfrom(state.socket,input.data(),input.size(),0,nullptr,nullptr);
			if(received < 0)
			{
				if(errno == EINTR) { --count; continue; }
				if(errno == EAGAIN || errno == EWOULDBLOCK) break;
				return state.fail("recvfrom",errno);
			}
			if(received == 0 || received > MAX_UDP_PACKET_SIZE)
			{
				state.fail("Oversized or empty UDP datagram",EMSGSIZE);
				continue;
			}
			state.receive.assign(input.begin(),input.begin()+received);
			DataRecv(); // Bad datagrams are dropped; the live socket remains usable.
		}
	}
	if(descriptor.revents & POLLOUT) return state.flush();
	return true;
}

DWORD CSocketManagerUdp::ServerRecvThread(CSocketManagerUdp*)
{
	errno = ENOTSUP;
	return ENOTSUP;
}

WORD CSocketManagerUdp::NativePort()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return m_native->port;
}

std::string CSocketManagerUdp::NativeLastError()
{
	std::lock_guard<std::recursive_mutex> lock(m_native->mutex);
	return m_native->error;
}
