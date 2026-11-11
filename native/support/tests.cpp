#include "socket_manager.hpp"
#include <iostream>
#include <stdexcept>

namespace {
void Require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
template<class Action>void Reject(Action action,const char* message){bool rejected=false;try{action();}catch(const std::exception&){rejected=true;}Require(rejected,message);}
void CountOnlyCompletePackets(){
 mu::support::FrameQueue queue;
 std::vector<BYTE> login(44,0);login[0]=0xC1;login[1]=44;login[2]=1;
 queue.Append(login.data(),4);
 Require(queue.PacketCount()==0&&queue.HasPartial(),"partial header/payload is not a queued packet");
 queue.Append(login.data()+4,login.size()-4);
 Require(queue.PacketCount()==1&&!queue.HasPartial(),"44 login bytes must count as exactly one packet");
 const BYTE list[]{0xC2,0,4,0x01};
 queue.Append(list,sizeof(list));
 Require(queue.PacketCount()==2,"mixed variable-width packet count");
 Require(queue.Take()==login&&queue.PacketCount()==1,"frame order and exact removal count");
 Require(queue.Take()==std::vector<BYTE>(list,list+sizeof(list))&&queue.BufferedBytes()==0,"C2 complete frame and released buffer");
}
void SplitAndDrainWithoutNewBytes(){
 mu::support::FrameQueue queue;
 const std::vector<BYTE> one{0xC1,4,0xFA,7};
 for(BYTE byte:one)queue.Append(&byte,1);
 Require(queue.PacketCount()==1,"every-byte split framing");
 queue.Reset();
 std::vector<BYTE> batch;
 for(int i=0;i<200;++i)batch.insert(batch.end(),one.begin(),one.end());
 queue.Append(batch.data(),batch.size());
 std::size_t dispatched=0;
 auto dispatch=[&](std::vector<BYTE> packet){Require(packet==one,"coalesced frame contents");++dispatched;return true;};
 Require(queue.Drain(64,dispatch)==64&&queue.PacketCount()==136,"first fair dispatch budget leaves exact ready count");
 // No new Append/POLLIN is needed between these dispatch cycles.
 Require(queue.Drain(64,dispatch)==64&&queue.PacketCount()==72,"second cycle drains existing work without new input");
 Require(queue.Drain(64,dispatch)==64&&queue.PacketCount()==8,"third bounded cycle");
 Require(queue.Drain(64,dispatch)==8&&dispatched==200&&queue.PacketCount()==0,"pending frames cannot remain stuck behind a receive event");
}
void LimitsAndDisconnectDuringDispatch(){
 const BYTE shortC2[]{0xC2,0,3,1},encrypted[]{0xC3,3,1},oversized[]{0xC2,0x20,0x01,1};
 for(const auto& bytes:{std::vector<BYTE>(shortC2,shortC2+4),std::vector<BYTE>(encrypted,encrypted+3),std::vector<BYTE>(oversized,oversized+4)}){
  mu::support::FrameQueue queue;Reject([&]{queue.Append(bytes.data(),bytes.size());},"invalid support type/header/length must be rejected");
 }
 const BYTE two[]{0xC1,3,0xFA,0xC1,3,0xFA};
 mu::support::FrameQueue limitedPackets(1,100);Reject([&]{limitedPackets.Append(two,sizeof(two));},"packet queue is bounded");
 mu::support::FrameQueue limitedBytes(10,5);Reject([&]{limitedBytes.Append(two,sizeof(two));},"receive bytes are bounded");
 mu::support::FrameQueue disconnected;
 disconnected.Append(two,sizeof(two));
 Require(disconnected.Drain(64,[&](std::vector<BYTE>){disconnected.Reset();return false;})==1,"disconnect/generation change stops dispatch immediately");
 Require(disconnected.PacketCount()==0,"disconnected session clears old queued frames");
}
}
int main(){
 try{CountOnlyCompletePackets();SplitAndDrainWithoutNewBytes();LimitsAndDisconnectDuringDispatch();std::cout<<"Native support framing/queue fairness tests passed; no sockets or database connections opened.\n";return 0;}
 catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
