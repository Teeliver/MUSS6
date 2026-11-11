#include "stdafx.h"
#include "SocketManager.h"
#include "ServerManager.h"
#include "AllowableIpList.h"
#include "Util.h"
#if defined(MU_NATIVE_JOIN)
#include "JoinServerProtocol.h"
#else
#include "DataServerProtocol.h"
#endif
#include <poll.h>
#include <fcntl.h>

namespace {
bool PrepareSocket(int socket){
 const int flags=fcntl(socket,F_GETFL,0),descriptorFlags=fcntl(socket,F_GETFD,0);
 if(flags<0||descriptorFlags<0||fcntl(socket,F_SETFL,flags|O_NONBLOCK)<0||fcntl(socket,F_SETFD,descriptorFlags|FD_CLOEXEC)<0)return false;
#ifdef SO_NOSIGPIPE
 const int enabled=1;if(setsockopt(socket,SOL_SOCKET,SO_NOSIGPIPE,&enabled,sizeof(enabled))<0)return false;
#endif
 return true;
}
}
struct CSocketManager::State{
 struct Peer{
  int socket=-1;bool eof=false;
  mu::support::FrameQueue input{};
  std::deque<std::vector<BYTE>> output;
  std::size_t sent=0,queued=0;
 };
 int listen=-1;std::string host="127.0.0.1";
 std::array<Peer,MAX_SERVER> peers;
 std::array<std::uint64_t,MAX_SERVER> generations{};
 int next=0;
};
CSocketManager gSocketManager;
CSocketManager::CSocketManager():state_(std::make_unique<State>()){}
CSocketManager::~CSocketManager(){Clean();}
void CSocketManager::SetBindAddress(const std::string& host){if(state_->listen>=0)throw std::runtime_error("Listener already started");state_->host=host;}
bool CSocketManager::Start(WORD port){
 if(state_->listen>=0||!port)return false;
 const int socket=::socket(AF_INET,SOCK_STREAM,0);if(socket<0)return false;
 sockaddr_in address{};address.sin_family=AF_INET;address.sin_port=htons(port);
 const int yes=1;setsockopt(socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
 if(inet_pton(AF_INET,state_->host.c_str(),&address.sin_addr)!=1||!PrepareSocket(socket)||::bind(socket,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0||::listen(socket,20)<0){::close(socket);return false;}
 state_->listen=socket;return true;
}
bool CSocketManager::Current(int index,int socket,std::uint64_t generation)const{
 return SERVER_RANGE(index)&&state_->generations[index]==generation&&state_->peers[index].socket==socket&&
  gServerManager[index].CheckState()&&gServerManager[index].m_socket==socket;
}
void CSocketManager::Disconnect(int index){
 if(!SERVER_RANGE(index))return;
 auto& peer=state_->peers[index];const int socket=peer.socket;
 if(socket<0)return;
 const bool managerOwnsSocket=gServerManager[index].CheckState()&&gServerManager[index].m_socket==socket;
 ::close(socket);
 peer=State::Peer{};++state_->generations[index];
 if(managerOwnsSocket)gServerManager[index].DelServer();
}
void CSocketManager::Clean(){
 // Unstarted/preflight instances own no sockets: no account cleanup or database
 // operation may be triggered by a static destructor on that path.
 for(int i=0;i<MAX_SERVER;++i)if(state_->peers[i].socket>=0)Disconnect(i);
 if(state_->listen>=0)::close(state_->listen);
 state_->listen=-1;
}
DWORD CSocketManager::GetQueueSize(){
 DWORD count=0;
 for(const auto& peer:state_->peers)count+=static_cast<DWORD>(peer.input.PacketCount());
 return count;
}
bool CSocketManager::DataSend(int index,BYTE* data,int size){
 if(!SERVER_RANGE(index)||!gServerManager[index].CheckState()||!data||size<3||size>MAX_MAIN_PACKET_SIZE)return false;
 auto& peer=state_->peers[index];
 if(peer.socket<0||peer.socket!=gServerManager[index].m_socket)return false;
 if(peer.queued+std::size_t(size)>MAX_SIDE_PACKET_SIZE){Disconnect(index);return false;}
 peer.output.emplace_back(data,data+size);peer.queued+=std::size_t(size);return true;
}
void CSocketManager::ReadPeer(int index){
 auto& peer=state_->peers[index];
 BYTE chunk[MAX_MAIN_PACKET_SIZE];
 const ssize_t count=::recv(peer.socket,chunk,sizeof(chunk),0);
 if(count<0){if(errno!=EINTR&&errno!=EAGAIN&&errno!=EWOULDBLOCK)Disconnect(index);return;}
 if(count==0){peer.eof=true;return;}
 try{peer.input.Append(chunk,static_cast<std::size_t>(count));}
 catch(const std::exception& error){LogAdd(LOG_RED,"[SocketTCP] Peer framing rejected: %s",error.what());Disconnect(index);}
}
void CSocketManager::WritePeer(int index){
 auto& peer=state_->peers[index];
 const int socket=peer.socket;const auto generation=state_->generations[index];
 std::size_t written=0;
 constexpr std::size_t budget=65536;
#ifdef MSG_NOSIGNAL
 constexpr int flags=MSG_NOSIGNAL;
#else
 constexpr int flags=0;
#endif
 while(Current(index,socket,generation)&&!peer.output.empty()&&written<budget){
  auto& packet=peer.output.front();
  const auto remaining=std::min(packet.size()-peer.sent,budget-written);
  const ssize_t count=::send(socket,packet.data()+peer.sent,remaining,flags);
  if(count<0){if(errno!=EAGAIN&&errno!=EWOULDBLOCK&&errno!=EINTR)Disconnect(index);break;}
  if(count==0){Disconnect(index);break;}
  const auto size=static_cast<std::size_t>(count);peer.sent+=size;peer.queued-=size;written+=size;
  if(peer.sent==packet.size()){peer.output.pop_front();peer.sent=0;}
 }
}
void CSocketManager::DrainPending(){
 constexpr std::size_t perPeerBudget=64;
 constexpr std::size_t totalBudget=2048;
 std::size_t consumed=0;
 const int first=state_->next;
 for(int step=0;step<MAX_SERVER&&consumed<totalBudget;++step){
  const int index=(first+step)%MAX_SERVER;
  auto& peer=state_->peers[index];
  const int socket=peer.socket;const auto generation=state_->generations[index];
  if(socket<0||!Current(index,socket,generation))continue;
  if(peer.eof&&peer.input.HasPartial()){Disconnect(index);continue;}
  consumed+=peer.input.Drain(std::min(perPeerBudget,totalBudget-consumed),[&](std::vector<BYTE> bytes){
   // Raw C++ handlers expect aligned, bounded storage for their struct casts.
   alignas(8)std::array<BYTE,MAX_MAIN_PACKET_SIZE> packet{};
   std::memcpy(packet.data(),bytes.data(),bytes.size());
   const std::size_t header=bytes[0]==0xC1?3:4;
#if defined(MU_NATIVE_JOIN)
   JoinServerProtocolCore(index,packet[header-1],packet.data(),int(bytes.size()));
#else
   DataServerProtocolCore(index,packet[header-1],packet.data(),int(bytes.size()));
#endif
   return Current(index,socket,generation);
  });
 }
 state_->next=(first+1)%MAX_SERVER;
}
void CSocketManager::Poll(int timeout){
 if(state_->listen<0)throw std::runtime_error("Server listener not started");
 std::vector<pollfd> descriptors{{state_->listen,POLLIN,0}};
 std::vector<int> indices{-1};std::vector<std::uint64_t> generations{0};
 bool pending=false;
 for(int i=0;i<MAX_SERVER;++i){
  auto& peer=state_->peers[i];
  if(peer.socket<0)continue;
  if(!Current(i,peer.socket,state_->generations[i])){Disconnect(i);continue;}
  pending=pending||peer.input.PacketCount()>0;
  const short events=static_cast<short>((!peer.eof&&peer.input.WantsRead()?POLLIN:0)|(peer.output.empty()?0:POLLOUT));
  descriptors.push_back({peer.socket,events,0});indices.push_back(i);generations.push_back(state_->generations[i]);
 }
 const int ready=::poll(descriptors.data(),descriptors.size(),pending?0:timeout);
 if(ready<0&&errno!=EINTR)throw std::runtime_error(std::string("poll failed: ")+std::strerror(errno));
 if(ready>=0&&(descriptors[0].revents&POLLIN))for(int attempt=0;attempt<MAX_SERVER;++attempt){
  sockaddr_in address{};socklen_t length=sizeof(address);const int socket=::accept(state_->listen,reinterpret_cast<sockaddr*>(&address),&length);
  if(socket<0){if(errno==EINTR)continue;break;}
  char ip[16]{};const int index=GetFreeServerIndex();
  if(index<0||!inet_ntop(AF_INET,&address.sin_addr,ip,sizeof(ip))||!gAllowableIpList.CheckAllowableIp(ip)||!PrepareSocket(socket)){::close(socket);continue;}
  state_->peers[index]=State::Peer{};state_->peers[index].socket=socket;++state_->generations[index];
  gServerManager[index].AddServer(index,ip,socket);
 }
 if(ready>=0)for(std::size_t n=1;n<descriptors.size();++n){
  const int index=indices[n],socket=descriptors[n].fd;
  if(!Current(index,socket,generations[n]))continue;
  if(descriptors[n].revents&(POLLERR|POLLNVAL)){Disconnect(index);continue;}
  // HUP can arrive while readable bytes remain in the kernel. Read until recv
  // returns zero on later cycles; do not declare EOF and discard a batch tail.
  if((descriptors[n].revents&POLLIN)||((descriptors[n].revents&POLLHUP)&&!state_->peers[index].eof&&state_->peers[index].input.WantsRead()))ReadPeer(index);
 }
 // This is unconditional: frames left by a dispatch budget are processed on the
 // next Poll even when the kernel has no new readable bytes for that peer.
 DrainPending();
 for(int i=0;i<MAX_SERVER;++i){
  auto& peer=state_->peers[i];const int socket=peer.socket;const auto generation=state_->generations[i];
  if(socket<0||!Current(i,socket,generation))continue;
  WritePeer(i);
  if(Current(i,socket,generation)&&peer.eof&&!peer.input.PacketCount()&&!peer.input.HasPartial()&&peer.output.empty())Disconnect(i);
 }
}
