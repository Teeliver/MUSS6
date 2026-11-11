#include "stdafx.h"
#include "SocketManager.h"
#include "BlackList.h"
#include "ClientManager.h"
#include "ConnectServerProtocol.h"
#include "IpManager.h"
#include "Util.h"
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

CSocketManager gSocketManager;
namespace {
bool Nonblocking(int socket) {
  const int flags=fcntl(socket,F_GETFL,0);
  if(flags<0 || fcntl(socket,F_SETFL,flags|O_NONBLOCK)<0) return false;
  if(fcntl(socket,F_SETFD,FD_CLOEXEC)<0) return false;
#ifdef SO_NOSIGPIPE
  const int enabled=1;
  if(setsockopt(socket,SOL_SOCKET,SO_NOSIGPIPE,&enabled,sizeof(enabled))<0) return false;
#endif
  return true;
}
BYTE PacketHead(const std::vector<BYTE>& bytes) { return bytes[bytes[0]==0xC1?2:3]; }
void ValidateRequest(const std::vector<BYTE>& bytes) {
  if(PacketHead(bytes)!=0xF4) return; // Original core ignores unknown heads.
  if(bytes[0]!=0xC1 || bytes.size()<4) throw std::runtime_error("Connect F4 request has invalid header");
  if(bytes[3]==0x03 && bytes.size()!=sizeof(PMSG_SERVER_INFO_RECV)) throw std::runtime_error("Connect F4:03 has invalid size");
  if(bytes[3]==0x06 && bytes.size()!=sizeof(PMSG_SERVER_LIST_RECV)) throw std::runtime_error("Connect F4:06 has invalid size");
}
}
CSocketManager::CSocketManager()=default;
CSocketManager::~CSocketManager() { Clean(); }
void CSocketManager::SetBindAddress(const std::string& address) { if(started_) throw std::runtime_error("Cannot change active Connect bind address"); bindAddress_=address; }
bool CSocketManager::Start(WORD port) {
  if(started_ || !port) return false;
  listen_=::socket(AF_INET,SOCK_STREAM,0);
  if(listen_<0) { LogAdd(LOG_RED,"Connect socket failed: %s",std::strerror(errno)); return false; }
  const int enabled=1;
  setsockopt(listen_,SOL_SOCKET,SO_REUSEADDR,&enabled,sizeof(enabled));
  sockaddr_in address{}; address.sin_family=AF_INET; address.sin_port=htons(port);
  if(inet_pton(AF_INET,bindAddress_.c_str(),&address.sin_addr)!=1 || !Nonblocking(listen_) ||
     ::bind(listen_,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0 || ::listen(listen_,5)<0) {
    LogAdd(LOG_RED,"Connect TCP start failed: %s",std::strerror(errno)); ::close(listen_); listen_=-1; return false;
  }
  started_=true;
  LogAdd(LOG_BLUE,"Native Connect TCP listening at %s:%u",bindAddress_.c_str(),unsigned(port));
  return true;
}
void CSocketManager::Clean() {
  if(!started_ && listen_<0) return;
  for(int index=0;index<MAX_CLIENT;++index) Disconnect(index);
  if(listen_>=0) ::close(listen_);
  listen_=-1; started_=false; queue_.clear();
}
DWORD CSocketManager::GetQueueSize() { return static_cast<DWORD>(queue_.size()); }
void CSocketManager::Disconnect(int index) {
  if(!CLIENT_RANGE(index) || !gClientManager[index].CheckState()) return;
  auto& client=gClientManager[index];
  ::close(client.m_socket);
  client.m_IoRecvContext->decoder.Reset();
  client.m_IoSendContext->pending.clear(); client.m_IoSendContext->offset=0;
  client.DelClient();
  ++generations_[index];
}
bool CSocketManager::DataSend(int index,BYTE* message,int size) {
  if(!CLIENT_RANGE(index) || !gClientManager[index].CheckState() || size<3 || size>MAX_MAIN_PACKET_SIZE) return false;
  auto& output=*gClientManager[index].m_IoSendContext;
  const std::size_t pending=output.pending.size()-output.offset;
  if(pending+static_cast<std::size_t>(size)>MAX_MAIN_PACKET_SIZE+MAX_SIDE_PACKET_SIZE) {
    LogAdd(LOG_RED,"Connect output buffer exceeded at index %d",index); Disconnect(index); return false;
  }
  if(output.offset) { output.pending.erase(output.pending.begin(),output.pending.begin()+output.offset); output.offset=0; }
  output.pending.insert(output.pending.end(),message,message+size);
  return true;
}
void CSocketManager::Accept() {
  for(int attempt=0;attempt<MAX_CLIENT;++attempt) {
    sockaddr_in address{}; socklen_t length=sizeof(address);
    const int socket=::accept(listen_,reinterpret_cast<sockaddr*>(&address),&length);
    if(socket<0) { if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR) LogAdd(LOG_RED,"Connect accept failed: %s",std::strerror(errno)); return; }
    char ip[INET_ADDRSTRLEN]{};
    const int index=GetFreeClientIndex();
    if(!inet_ntop(AF_INET,&address.sin_addr,ip,sizeof(ip)) || index<0 || !gBlackList.CheckIpAddress(ip) || !gIpManager.CheckIpAddress(ip) || !Nonblocking(socket)) { ::close(socket); continue; }
    ++generations_[index];
    gClientManager[index].AddClient(index,ip,socket); // Sends original init/name packets.
  }
}
void CSocketManager::Receive(int index) {
  auto& client=gClientManager[index];
  for(int attempt=0;attempt<32 && client.CheckState();++attempt) {
    auto& decoder=client.m_IoRecvContext->decoder;
    BYTE bytes[MAX_MAIN_PACKET_SIZE];
    const auto capacity=decoder.Capacity();
    if(!capacity) { Disconnect(index); return; }
    const ssize_t size=::recv(client.m_socket,bytes,capacity,0);
    if(size==0) { Disconnect(index); return; }
    if(size<0) { if(errno==EINTR) continue; if(errno!=EAGAIN && errno!=EWOULDBLOCK) Disconnect(index); return; }
    try {
      decoder.Feed(bytes,static_cast<std::size_t>(size),[&](std::vector<BYTE> packet) {
        ValidateRequest(packet);
        if(++client.m_PacketCount>MaxPacketPerSecond) throw std::runtime_error("Connect packets-per-second limit exceeded");
        if(queue_.size()>=MAX_QUEUE_SIZE) throw std::runtime_error("Connect protocol queue limit exceeded");
        queue_.push_back({index,generations_[index],std::move(packet)});
      });
    } catch(const std::exception& error) { LogAdd(LOG_RED,"Connect peer rejected: %s",error.what()); Disconnect(index); return; }
  }
}
void CSocketManager::Flush(int index) {
  auto& client=gClientManager[index];
  auto& output=*client.m_IoSendContext;
  if(output.pending.size()==output.offset) return;
#ifdef MSG_NOSIGNAL
  constexpr int flags=MSG_NOSIGNAL;
#else
  constexpr int flags=0;
#endif
  const ssize_t sent=::send(client.m_socket,output.pending.data()+output.offset,output.pending.size()-output.offset,flags);
  if(sent<0) { if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR) Disconnect(index); return; }
  if(sent==0) { Disconnect(index); return; }
  output.offset+=static_cast<std::size_t>(sent);
  if(output.offset==output.pending.size()) { output.pending.clear(); output.offset=0; }
}
void CSocketManager::Dispatch() {
  while(!queue_.empty()) {
    auto packet=std::move(queue_.front()); queue_.pop_front();
    if(!gClientManager[packet.index].CheckState() || packet.generation!=generations_[packet.index]) continue;
    ConnectServerProtocolCore(packet.index,PacketHead(packet.bytes),packet.bytes.data(),static_cast<int>(packet.bytes.size()));
  }
}
void CSocketManager::Poll(int timeoutMilliseconds) {
  if(!started_) throw std::runtime_error("Connect TCP server not started");
  std::vector<pollfd> descriptors{{listen_,POLLIN,0}};
  std::vector<int> indexes;
  for(int index=0;index<MAX_CLIENT;++index) if(gClientManager[index].CheckState()) {
    const auto& output=*gClientManager[index].m_IoSendContext;
    descriptors.push_back({gClientManager[index].m_socket,static_cast<short>(POLLIN|(output.pending.size()>output.offset?POLLOUT:0)),0}); indexes.push_back(index);
  }
  const int ready=::poll(descriptors.data(),descriptors.size(),timeoutMilliseconds);
  if(ready<0) { if(errno==EINTR) return; throw std::runtime_error(std::string("Connect poll failed: ")+std::strerror(errno)); }
  if(descriptors[0].revents&POLLIN) Accept();
  for(std::size_t i=1;i<descriptors.size();++i) {
    const int index=indexes[i-1];
    if(!gClientManager[index].CheckState() || gClientManager[index].m_socket!=descriptors[i].fd) continue;
    if(descriptors[i].revents&(POLLERR|POLLNVAL)) { Disconnect(index); continue; }
    if(descriptors[i].revents&POLLIN) Receive(index);
    if(gClientManager[index].CheckState() && descriptors[i].revents&POLLOUT) Flush(index);
    if(gClientManager[index].CheckState() && descriptors[i].revents&POLLHUP) Disconnect(index);
  }
  Dispatch();
}
