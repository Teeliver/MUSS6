#pragma once
#include "mu/platform.hpp"
#include <deque>
#define MAX_MAIN_PACKET_SIZE 8192
#define MAX_SIDE_PACKET_SIZE 262144
#define IO_RECV 0
#define IO_SEND 1
namespace mu::support {
// Shared, offline-testable framing used by the real support-service transport.
// Partial headers/payloads are not counted as queued protocol packets.
class FrameQueue {
 public:
  explicit FrameQueue(std::size_t packetLimit=2048,std::size_t byteLimit=MAX_SIDE_PACKET_SIZE):packetLimit_(packetLimit),byteLimit_(byteLimit){}
  void Reset(){tail_.clear();packets_.clear();packetBytes_=0;}
  std::size_t PacketCount()const{return packets_.size();}
  std::size_t BufferedBytes()const{return packetBytes_+tail_.size();}
  bool HasPartial()const{return !tail_.empty();}
  bool WantsRead()const{return PacketCount()<std::max<std::size_t>(1,packetLimit_/2)&&BufferedBytes()<byteLimit_/2;}
  void Append(const BYTE* bytes,std::size_t size){
   if(!size)return;
   if(!bytes||size>byteLimit_-BufferedBytes())throw std::runtime_error("Support receive buffer limit exceeded");
   tail_.insert(tail_.end(),bytes,bytes+size);
   std::size_t consumed=0;
   while(consumed<tail_.size()){
    const BYTE type=tail_[consumed];
    if(type!=0xC1&&type!=0xC2)throw std::runtime_error("Support frame type must be C1/C2");
    const std::size_t header=type==0xC1?3:4;
    if(tail_.size()-consumed<header)break;
    const std::size_t length=type==0xC1?tail_[consumed+1]:(std::size_t(tail_[consumed+1])<<8)|tail_[consumed+2];
    if(length<header||length>MAX_MAIN_PACKET_SIZE)throw std::runtime_error("Support frame length out of range");
    if(tail_.size()-consumed<length)break;
    if(packets_.size()>=packetLimit_)throw std::runtime_error("Support protocol packet queue limit exceeded");
    packets_.emplace_back(tail_.begin()+consumed,tail_.begin()+consumed+length);
    packetBytes_+=length;consumed+=length;
   }
   tail_.erase(tail_.begin(),tail_.begin()+consumed);
  }
  std::vector<BYTE> Take(){
   if(packets_.empty())throw std::runtime_error("No complete support packet queued");
   auto packet=std::move(packets_.front());packets_.pop_front();packetBytes_-=packet.size();return packet;
  }
  template<class Dispatch>std::size_t Drain(std::size_t budget,Dispatch dispatch){
   std::size_t count=0;
   while(count<budget&&!packets_.empty()){auto packet=Take();++count;if(!dispatch(std::move(packet)))break;}
   return count;
  }
 private:
  std::size_t packetLimit_,byteLimit_,packetBytes_=0;
  std::vector<BYTE> tail_;
  std::deque<std::vector<BYTE>> packets_;
};
}
struct IO_MAIN_BUFFER{BYTE buff[MAX_MAIN_PACKET_SIZE];int size;};
struct IO_SIDE_BUFFER{BYTE buff[MAX_SIDE_PACKET_SIZE];int size;};
struct IO_RECV_CONTEXT{WSAOVERLAPPED overlapped;WSABUF wsabuf;int IoType;int IoSize;IO_MAIN_BUFFER IoMainBuffer;};
struct IO_SEND_CONTEXT{WSAOVERLAPPED overlapped;WSABUF wsabuf;int IoType;int IoSize;IO_MAIN_BUFFER IoMainBuffer;IO_SIDE_BUFFER IoSideBuffer;};
class CSocketManager{
 public:
  CSocketManager();~CSocketManager();
  bool Start(WORD port);void Clean();
  bool DataSend(int index,BYTE* data,int size);void Disconnect(int index);
  DWORD GetQueueSize();void Poll(int timeout);
  void SetBindAddress(const std::string& host);
 private:
  struct State;std::unique_ptr<State> state_;
  bool Current(int index,int socket,std::uint64_t generation)const;
  void ReadPeer(int index);
  void WritePeer(int index);
  void DrainPending();
};
extern CSocketManager gSocketManager;
