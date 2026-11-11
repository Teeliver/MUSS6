#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <set>
#include <stdexcept>
#include <algorithm>
namespace mu::core {
using WireBytes=std::vector<std::uint8_t>;
inline void wire_require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct TransportKeys {std::uint8_t key1,key2;bool mhp;std::uint8_t mhp1,mhp2;};
// HackCheck byte transform, not C3/C4 SimpleModulus or packet XOR filtering.
inline WireBytes encode_transport(WireBytes bytes,TransportKeys k){
 for(auto& b:bytes){b=std::uint8_t((b+unsigned(k.key1)*k.key2)^k.key1);if(k.mhp)b=std::uint8_t((b+k.mhp2)^k.mhp1);}return bytes;
}
inline WireBytes decode_transport(WireBytes bytes,TransportKeys k){
 for(auto& b:bytes){if(k.mhp)b=std::uint8_t((b^k.mhp1)-k.mhp2);b=std::uint8_t((b^k.key1)-unsigned(k.key1)*k.key2);}return bytes;
}
struct ServerEntry {unsigned code,load,type;};
struct ServerEndpoint {std::string address;unsigned port;};
inline void connect_handshake(const WireBytes& bytes){wire_require(bytes==WireBytes({0xC1,4,0,1}),"ConnectServer handshake rejected");}
inline WireBytes server_list_request(){return {0xC1,4,0xF4,6};}
inline WireBytes server_endpoint_request(unsigned code){wire_require(code<=255,"F4:03 requires a one-byte server code");return {0xC1,5,0xF4,3,std::uint8_t(code)};}
inline std::vector<ServerEntry> server_list(const WireBytes& b){
 wire_require(b.size()>=7&&b.size()<=16384&&b[0]==0xC2&&((unsigned(b[1])<<8)|b[2])==b.size()&&b[3]==0xF4&&b[4]==6,"Invalid ConnectServer list frame");
 const unsigned count=(unsigned(b[5])<<8)|b[6];wire_require(b.size()==7+count*4,"Server-list count/length mismatch");
 std::vector<ServerEntry> result;std::set<unsigned> codes;
 for(unsigned i=0;i<count;i++){unsigned n=7+i*4,code=unsigned(b[n])|(unsigned(b[n+1])<<8);wire_require(codes.insert(code).second,"Duplicate server code");result.push_back({code,b[n+2],b[n+3]});}return result;
}
inline ServerEndpoint server_endpoint(const WireBytes& b){
 wire_require(b.size()==22&&b[0]==0xC1&&b[1]==22&&b[2]==0xF4&&b[3]==3,"Invalid GameServer endpoint frame");
 auto first=b.begin()+4,last=b.begin()+20,nul=std::find(first,last,0);wire_require(nul!=last&&nul!=first,"Invalid endpoint address field");
 for(auto i=first;i!=nul;i++)wire_require(*i>=33&&*i<=126,"Invalid endpoint address bytes");
 unsigned port=unsigned(b[20])|(unsigned(b[21])<<8);wire_require(port>0,"Invalid endpoint port");return {std::string(first,nul),port};
}
}
