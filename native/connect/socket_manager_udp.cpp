#include "stdafx.h"
#include "SocketManagerUdp.h"
#include "ServerList.h"
#include "Util.h"
#include "datagram.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

CSocketManagerUdp gSocketManagerUdp;
namespace mu::connect {
std::vector<std::vector<BYTE>> DecodeLiveDatagram(const BYTE* bytes,std::size_t size) {
  if(!size || size>MAX_UDP_PACKET_SIZE) throw std::runtime_error("Connect UDP datagram size out of range");
  std::vector<std::vector<BYTE>> packets;
  for(std::size_t offset=0;offset<size;) {
    const BYTE type=bytes[offset];
    if(type!=0xC1 && type!=0xC2) throw std::runtime_error("Connect UDP invalid frame type");
    const std::size_t header=type==0xC1?3:4;
    if(size-offset<header) throw std::runtime_error("Connect UDP incomplete header");
    const std::size_t length=type==0xC1?bytes[offset+1]:(std::size_t(bytes[offset+1])<<8)|bytes[offset+2];
    if(length<header || length>size-offset) throw std::runtime_error("Connect UDP incomplete or invalid frame");
    const BYTE head=bytes[offset+header-1];
    if(head==0xA1 || head==0xA2) {
      const auto expected=head==0xA1?sizeof(SDHP_GAME_SERVER_LIVE_RECV):sizeof(SDHP_JOIN_SERVER_LIVE_RECV);
      if(type!=0xC1 || length!=expected) throw std::runtime_error("Connect UDP heartbeat ABI size mismatch");
    }
    packets.emplace_back(bytes+offset,bytes+offset+length);
    offset+=length;
  }
  return packets;
}
}
CSocketManagerUdp::CSocketManagerUdp()=default;
CSocketManagerUdp::~CSocketManagerUdp() { Clean(); }
void CSocketManagerUdp::SetBindAddress(const std::string& address) { if(socket_>=0) throw std::runtime_error("Cannot change active Connect UDP bind address"); bindAddress_=address; }
bool CSocketManagerUdp::Start(WORD port) {
  if(socket_>=0 || !port) return false;
  socket_=::socket(AF_INET,SOCK_DGRAM,0);
  if(socket_<0) { LogAdd(LOG_RED,"Connect UDP socket failed: %s",std::strerror(errno)); return false; }
  sockaddr_in address{}; address.sin_family=AF_INET; address.sin_port=htons(port);
  const int flags=fcntl(socket_,F_GETFL,0);
  if(inet_pton(AF_INET,bindAddress_.c_str(),&address.sin_addr)!=1 || flags<0 || fcntl(socket_,F_SETFL,flags|O_NONBLOCK)<0 ||
     fcntl(socket_,F_SETFD,FD_CLOEXEC)<0 || ::bind(socket_,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0) {
    LogAdd(LOG_RED,"Connect UDP start failed: %s",std::strerror(errno)); Clean(); return false;
  }
  LogAdd(LOG_BLUE,"Native Connect UDP listening at %s:%u",bindAddress_.c_str(),unsigned(port));
  return true;
}
void CSocketManagerUdp::Clean() { if(socket_>=0) ::close(socket_); socket_=-1; }
void CSocketManagerUdp::Poll() {
  if(socket_<0) throw std::runtime_error("Connect UDP server not started");
  for(int attempt=0;attempt<64;++attempt) {
    BYTE bytes[MAX_UDP_PACKET_SIZE]; sockaddr_in source{};
    iovec vector{bytes,sizeof(bytes)};
    msghdr message{}; message.msg_name=&source; message.msg_namelen=sizeof(source); message.msg_iov=&vector; message.msg_iovlen=1;
    const ssize_t size=::recvmsg(socket_,&message,0);
    if(size<0) { if(errno==EINTR) continue; if(errno!=EAGAIN && errno!=EWOULDBLOCK) LogAdd(LOG_RED,"Connect UDP receive failed: %s",std::strerror(errno)); return; }
    if(message.msg_flags&MSG_TRUNC) { LogAdd(LOG_RED,"Connect UDP rejected oversized datagram"); continue; }
    try {
      for(auto& packet:mu::connect::DecodeLiveDatagram(bytes,static_cast<std::size_t>(size))) {
        const BYTE head=packet[packet[0]==0xC1?2:3];
        gServerList.ServerProtocolCore(head,packet.data(),static_cast<int>(packet.size()));
      }
    } catch(const std::exception& error) { LogAdd(LOG_RED,"Connect UDP rejected packet: %s",error.what()); }
  }
}
