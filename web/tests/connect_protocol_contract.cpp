#include "mu/connect_protocol.hpp"
#include "mu/game_profile.hpp"
#include "mu/client_identity.hpp"
#include <iostream>
#ifdef MU_COMPARE_SERVER_SOURCE
#include "transport_reference.hpp"
#include "packet_filter_reference.hpp"
#endif
using namespace mu::core;
void check(bool ok){if(!ok)throw std::runtime_error("Contract failed");}
template<class F>void rejects(F f){bool error=false;try{f();}catch(const std::runtime_error&){error=true;}check(error);}
int main(){
 connect_handshake({0xC1,4,0,1});rejects([]{connect_handshake({0xC1,4,0,0});});
 check(server_list_request()==WireBytes({0xC1,4,0xF4,6}));check(server_endpoint_request(255)==WireBytes({0xC1,5,0xF4,3,255}));rejects([]{server_endpoint_request(256);});
 WireBytes list={0xC2,0,15,0xF4,6,0,2,1,0,50,0xCC,0,1,99,0xCC};
 auto entries=server_list(list);check(entries.size()==2&&entries[0].code==1&&entries[1].code==256&&entries[1].load==99);
 check(server_list({0xC2,0,7,0xF4,6,0,0}).empty());
 auto duplicate=list;duplicate[11]=1;duplicate[12]=0;rejects([&]{server_list(duplicate);});
 for(std::size_t n=0;n<list.size();n++)rejects([&]{server_list(WireBytes(list.begin(),list.begin()+n));});
 auto count=list;count[6]=3;rejects([&]{server_list(count);});
 WireBytes endpoint(22,0);endpoint[0]=0xC1;endpoint[1]=22;endpoint[2]=0xF4;endpoint[3]=3;
 std::string address="127.0.0.1";std::copy(address.begin(),address.end(),endpoint.begin()+4);endpoint[20]=0x34;endpoint[21]=0x12;
 auto parsed=server_endpoint(endpoint);check(parsed.address==address&&parsed.port==4660);
 auto bad=endpoint;std::fill(bad.begin()+4,bad.begin()+20,'x');rejects([&]{server_endpoint(bad);});
 bad=endpoint;bad[20]=bad[21]=0;rejects([&]{server_endpoint(bad);});
 WireBytes all;for(unsigned i=0;i<256;i++)all.push_back(i);
 for(auto keys:std::vector<TransportKeys>{{0,0,false,0,0},{0x12,0x34,false,0,0},{0x12,0x34,true,0x56,0x78},{255,255,true,255,255}}){
  auto encoded=encode_transport(all,keys);check(decode_transport(encoded,keys)==all);
#ifdef MU_COMPARE_SERVER_SOURCE
  server_reference::EncDecKey1=keys.key1;server_reference::EncDecKey2=keys.key2;
  server_reference::gServerInfo.m_ServerEncDecKey1=keys.mhp?keys.mhp1:0;
  server_reference::gServerInfo.m_ServerEncDecKey2=keys.mhp?keys.mhp2:0;
  auto original=all;server_reference::EncryptData(original.data(),int(original.size()));check(original==encoded);
  server_reference::DecryptData(original.data(),int(original.size()));check(original==all);
#endif
  for(std::size_t split=0;split<=all.size();split++){
   auto left=encode_transport(WireBytes(all.begin(),all.begin()+split),keys),right=encode_transport(WireBytes(all.begin()+split,all.end()),keys);left.insert(left.end(),right.begin(),right.end());check(left==encoded);
  }
 }
 check(encode_transport({0,1,127,128,255},{0x12,0x34,true,0x56,0x78})==WireBytes({0x64,0x65,0xfb,0xe4,0x7b}));
 for(const auto& serial:std::vector<std::string>{"1234567890123456","ABCDEFGHIJKLMNOP","zzzzzzzzzzzzzzzz"})for(const auto& raw:std::vector<std::array<unsigned,2>>{{0,0},{1,0},{0,255},{255,255}}){
  auto derived=derive_transport_keys(serial,raw[0],raw[1]);
#ifdef MU_COMPARE_SERVER_SOURCE
  std::copy(serial.begin(),serial.end(),server_reference::gServerInfo.m_ServerSerial);
  server_reference::gServerInfo.m_ServerSerial[16]=0;
  server_reference::gServerInfo.m_ServerEncDecKey1=raw[0];server_reference::gServerInfo.m_ServerEncDecKey2=raw[1];
  server_reference::InitHackCheck();
  check(derived.key1==server_reference::EncDecKey1&&derived.key2==server_reference::EncDecKey2);
  check(derived.mhp1==server_reference::gServerInfo.m_ServerEncDecKey1&&derived.mhp2==server_reference::gServerInfo.m_ServerEncDecKey2);
#endif
  check(decode_transport(encode_transport(all,derived),derived)==all);
 }
 rejects([]{derive_transport_keys("short",0,0);});rejects([]{derive_transport_keys("1234567890123456",256,0);});
 WireBytes greeting={0xC1,14,0xF1,0,1,0x2E,0x7C,49,48,50,48,48,0x34,0x12};
 auto g=game_greeting(greeting);check(g.index==11900&&g.serverCode==4660&&g.encodedVersion[0]==49);
 for(std::size_t n=0;n<greeting.size();n++)rejects([&]{game_greeting(WireBytes(greeting.begin(),greeting.begin()+n));});
 auto denied=greeting;denied[4]=0;rejects([&]{game_greeting(denied);});
 const std::string identity="01234567-89ABCDEF-01234567-89ABCDEF-01234567";
 auto identityPacket=hardware_identity_packet(identity);check(identityPacket.size()==49&&identityPacket[3]==9&&identityPacket[48]==0);
 check(std::string(identityPacket.begin()+4,identityPacket.begin()+48)==identity);
 for(unsigned position=0;position<44;position++){auto invalid=identity;invalid[position]='!';rejects([&]{hardware_identity_packet(invalid);});}
 rejects([]{hardware_identity_packet("short");});
 auto filtered=filter_plain_packet(identityPacket,true);check(filter_plain_packet(filtered,false)==identityPacket);check(filtered[2]==0xF3&&filtered[3]==(9^0xF3^0xFE));
#ifdef MU_COMPARE_SERVER_SOURCE
 packet_reference::MainFilter mainFilter;std::copy(identityPacket.begin(),identityPacket.end(),mainFilter.m_buff);mainFilter.XorData(3,49);
 check(WireBytes(mainFilter.m_buff,mainFilter.m_buff+49)==filtered);
 packet_reference::ServerFilter serverFilter;std::copy(filtered.begin(),filtered.end(),serverFilter.m_buff);serverFilter.XorData(48,2);
 check(WireBytes(serverFilter.m_buff,serverFilter.m_buff+49)==identityPacket);
#endif
 for(unsigned size:{4u,255u,256u,2047u}){
  WireBytes packet(size,0xA5);packet[0]=0xC2;packet[1]=size>>8;packet[2]=size&255;
  check(filter_plain_packet(filter_plain_packet(packet,true),false)==packet);
 }
 rejects([]{filter_plain_packet({0xC3,4,0xF1,1},true);});rejects([]{filter_plain_packet({0xC1,5,0xF3,9},true);});
 for(auto keys:std::vector<TransportKeys>{{1,2,false,0,0},{0x12,0x34,true,0x56,0x78}}){
  auto wire=encode_transport(filtered,keys);auto plain=filter_plain_packet(decode_transport(wire,keys),false);check(plain==identityPacket);
 }
 std::cout<<"connect-protocol contract passed\n";
}
