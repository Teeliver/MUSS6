#include "stdafx.h"
#include "BlackList.h"
#include "ConnectServerProtocol.h"
#include "IpManager.h"
#include "ServerList.h"
#include "SocketManagerUdp.h"
#include "datagram.hpp"
#include "frame_decoder.hpp"
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <stdexcept>

int MaxConnectionPerIP=2;
int MaxPacketPerSecond=20;
int MaxConnectionIdle=30;
char CustomerName[32]{};

static_assert(sizeof(BYTE)==1 && sizeof(WORD)==2 && sizeof(DWORD)==4 && sizeof(QWORD)==8,"Windows protocol integer widths");
static_assert(sizeof(PBMSG_HEAD)==3 && sizeof(PSBMSG_HEAD)==4 && sizeof(PWMSG_HEAD)==4 && sizeof(PSWMSG_HEAD)==5,"Protocol headers");
static_assert(sizeof(PMSG_SERVER_INIT_SEND)==4 && sizeof(PMSG_SERVER_INFO_RECV)==5,"Connect short packets");
static_assert(sizeof(PMSG_SERVER_INFO_SEND)==22 && offsetof(PMSG_SERVER_INFO_SEND,ServerPort)==20,"Server address response");
static_assert(sizeof(PMSG_SERVER_LIST_SEND)==7 && sizeof(PMSG_SERVER_LIST)==4,"Server list ABI");
static_assert(sizeof(PMSG_SERVER_NAME_LIST_SEND)==6 && sizeof(PMSG_SERVER_NAME)==34,"Server name ABI");
static_assert(sizeof(SDHP_GAME_SERVER_LIVE_RECV)==16 && offsetof(SDHP_GAME_SERVER_LIVE_RECV,ServerCode)==4,"A1 Windows padding");
static_assert(sizeof(SDHP_JOIN_SERVER_LIVE_RECV)==8 && offsetof(SDHP_JOIN_SERVER_LIVE_RECV,QueueSize)==4,"A2 Windows padding");

namespace {
void Require(bool condition,const char* message) { if(!condition) throw std::runtime_error(message); }
template<class Action> void Reject(Action action,const char* message) { bool failed=false; try { action(); } catch(const std::exception&) { failed=true; } Require(failed,message); }
template<class Packet> std::vector<BYTE> Bytes(const Packet& packet) {
  const auto* start=reinterpret_cast<const BYTE*>(&packet); return {start,start+sizeof(packet)};
}
void TestFrames() {
  mu::connect::FrameDecoder decoder;
  const std::vector<BYTE> input{0xC1,4,0xF4,0x06,0xC1,5,0xF4,0x03,0,0xC2,0,4,0x55};
  std::vector<std::vector<BYTE>> received;
  const auto receive=[&](std::vector<BYTE> packet) { received.push_back(std::move(packet)); };
  for(const BYTE byte:input) decoder.Feed(&byte,1,receive);
  Require(received.size()==3 && received[0]==std::vector<BYTE>({0xC1,4,0xF4,0x06}) && received[2][0]==0xC2,"split/coalesced frame parsing");
  Require(decoder.Buffered()==0,"decoder retains no complete frames");
  decoder.Feed(input.data(),input.size(),receive); Require(received.size()==6,"coalesced request batch");
  const BYTE malformed[]{0xC2,0,3,0x55};
  Reject([&]{decoder.Feed(malformed,sizeof(malformed),receive);},"C2 length must fit complete header"); decoder.Reset();
  const BYTE encrypted[]{0xC3,3,0xF4};
  Reject([&]{decoder.Feed(encrypted,sizeof(encrypted),receive);},"Connect does not accept encrypted header types"); decoder.Reset();
  const BYTE oversized[]{0xC2,0x08,0x01,0x55};
  Reject([&]{decoder.Feed(oversized,sizeof(oversized),receive);},"Connect packet size bound");
}
void TestRegistry() {
  std::filesystem::current_path(MU_CONNECT_TESTDATA);
  char blacklist[]="BlackList.txt",servers[]="ServerList.dat";
  gBlackList.Load(blacklist); gServerList.Load(servers);
  char blocked[]="203.0.113.10",local[]="127.0.0.1";
  Require(!gBlackList.CheckIpAddress(blocked) && gBlackList.CheckIpAddress(local),"blacklist loader must not skip its first IP");
  Require(gIpManager.CheckIpAddress(local),"new IP below configured connection limit");
  gIpManager.InsertIpAddress(local); gIpManager.InsertIpAddress(local);
  Require(!gIpManager.CheckIpAddress(local),"per-IP capacity");
  gIpManager.RemoveIpAddress(local); Require(gIpManager.CheckIpAddress(local),"connection removal releases IP capacity");
  gIpManager.RemoveIpAddress(local);
  MaxConnectionPerIP=0; Require(!gIpManager.CheckIpAddress(local),"original zero-limit rejection"); MaxConnectionPerIP=2;

  BYTE output[2048]{}; int size=sizeof(PMSG_SERVER_LIST_SEND);
  Require(gServerList.GenerateServerList(output,&size)==0,"Join heartbeat gates discovery");
  SDHP_GAME_SERVER_LIVE_RECV game{}; game.header.set(0xA1,sizeof(game)); game.ServerCode=0; game.UserCount=25; game.UserTotal=100;
  SDHP_JOIN_SERVER_LIVE_RECV join{}; join.header.set(0xA2,sizeof(join)); join.QueueSize=100;
  auto batch=Bytes(game),second=Bytes(join); batch.insert(batch.end(),second.begin(),second.end());
  const auto decoded=mu::connect::DecodeLiveDatagram(batch.data(),batch.size()); Require(decoded.size()==2,"UDP packed heartbeat datagram");
  for(auto packet:decoded) gServerList.ServerProtocolCore(packet[2],packet.data(),static_cast<int>(packet.size()));
  size=sizeof(PMSG_SERVER_LIST_SEND); Require(gServerList.GenerateServerList(output,&size)==1,"original registry discovers live server");
  PMSG_SERVER_LIST entry{}; std::memcpy(&entry,output+sizeof(PMSG_SERVER_LIST_SEND),sizeof(entry));
  Require(entry.ServerCode==0 && entry.UserTotal==25 && entry.type==0xCC,"discovery load and ABI entry");
  join.QueueSize=101; gServerList.JCJoinServerLiveRecv(&join);
  size=sizeof(PMSG_SERVER_LIST_SEND); Require(gServerList.GenerateServerList(output,&size)==0,"Join queue above 100 blocks discovery");
  join.QueueSize=0; gServerList.JCJoinServerLiveRecv(&join);
  game.UserTotal=0; gServerList.GCGameServerLiveRecv(&game);
  Require(gServerList.GetServerListInfo(0)->UserTotal==100,"invalid zero capacity must not poison original percent calculation");
  auto partial=Bytes(game); partial.pop_back();
  Reject([&]{mu::connect::DecodeLiveDatagram(partial.data(),partial.size());},"UDP fragments must not be combined across datagrams");
  auto* state=gServerList.GetServerListInfo(0); state->ServerStateTime=GetTickCount()-DWORD(10001); gServerList.MainProc();
  Require(state->ServerState==0,"original 10-second live expiry");
}
}
int main() {
  try { TestFrames(); TestRegistry(); std::cout<<"Native Connect ABI, framing, datagram and original registry tests passed; no sockets opened.\n"; return 0; }
  catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
