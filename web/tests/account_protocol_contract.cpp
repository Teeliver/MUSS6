#include "mu/account_protocol.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_ACCOUNT_REFERENCE
#include "account_reference.hpp"
#include <cstddef>
static_assert(sizeof(account_reference::PMSG_CONNECT_ACCOUNT_RECV)==59);
static_assert(offsetof(account_reference::PMSG_CONNECT_ACCOUNT_RECV,TickCount)==34);
static_assert(offsetof(account_reference::PMSG_CONNECT_ACCOUNT_RECV,ClientVersion)==38);
static_assert(offsetof(account_reference::PMSG_CONNECT_ACCOUNT_RECV,ClientSerial)==43);
#endif
using namespace mu::core;
template<class F> void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}assert(failed);}
int main(){
 WireBytes version{1,2,3,4,5};std::string serial="1234567890123456";
 for(unsigned n=1;n<=10;n++){
  std::string account(n,'A'),password(n,'!');auto b=account_login_packet(account,password,0xFEDCBA98,version,serial);
  assert(b.size()==59&&b[0]==0xC3&&b[1]==59&&b[2]==0xF1&&b[3]==1);
  assert(b[34]==0x98&&b[35]==0xBA&&b[36]==0xDC&&b[37]==0xFE);
  for(unsigned i=24;i<34;i++)assert(b[i]==0);
#ifdef MU_ACCOUNT_REFERENCE
  char decoded[10]{};account_reference::PacketArgumentDecrypt(decoded,reinterpret_cast<char*>(b.data()+4),10);
  for(unsigned i=0;i<10;i++)assert(decoded[i]==(i<n?'A':0));
  account_reference::PacketArgumentDecrypt(decoded,reinterpret_cast<char*>(b.data()+14),10);
  for(unsigned i=0;i<10;i++)assert(decoded[i]==(i<n?'!':0));
#endif
  assert(std::equal(version.begin(),version.end(),b.begin()+38));assert(std::equal(serial.begin(),serial.end(),b.begin()+43));
 }
 for(unsigned i=0;i<256;i++)assert(account_login_result({0xC1,5,0xF1,1,std::uint8_t(i)})==i);
 for(const auto& a:{"","12345678901","bad name","bad_"})rejects([&]{account_login_packet(a,"pass",0,version,serial);});
 for(const auto& p:{"","12345678901","bad pass","\n"})rejects([&]{account_login_packet("Test",p,0,version,serial);});
 rejects([&]{account_login_packet("Test","pass",0,{},serial);});
 rejects([&]{account_login_packet("Test","pass",0,version,"short");});
 for(unsigned i=0;i<4;i++){auto b=WireBytes{0xC1,5,0xF1,1,1};b[i]^=1;rejects([&]{account_login_result(b);});}
 rejects([]{account_login_result({0xC1,5,0xF1,1});});
 std::cout<<"account protocol contract passed\n";
}
