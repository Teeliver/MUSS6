#pragma once
#include "connect_protocol.hpp"
#include <array>
#include <string>
namespace mu::core {
// EX603: 20-byte password wire field, but this server reads only its first 10 bytes.
inline WireBytes account_login_packet(const std::string& account,const std::string& password,
 std::uint32_t tick,const WireBytes& version,const std::string& serial) {
 wire_require(!account.empty()&&account.size()<=10,"Invalid account length");
 for(unsigned char c:account)wire_require((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9'),"Invalid account encoding");
 wire_require(!password.empty()&&password.size()<=10,"Unsupported EX603 password length");
 for(unsigned char c:password)wire_require(c>=33&&c<=126,"Invalid password encoding");
 wire_require(version.size()==5&&serial.size()==16,"Invalid login profile layout");
 for(unsigned char c:serial)wire_require(c>=33&&c<=126,"Invalid serial encoding");
 WireBytes out(59,0);out[0]=0xC3;out[1]=59;out[2]=0xF1;out[3]=1;
 constexpr unsigned char mask[]={0xFC,0xCF,0xAB};
 for(unsigned i=0;i<10;i++){out[4+i]=(i<account.size()?std::uint8_t(account[i]):0)^mask[i%3];out[14+i]=(i<password.size()?std::uint8_t(password[i]):0)^mask[i%3];}
 for(unsigned i=0;i<4;i++)out[34+i]=std::uint8_t(tick>>(i*8));
 std::copy(version.begin(),version.end(),out.begin()+38);std::copy(serial.begin(),serial.end(),out.begin()+43);return out;
}
inline unsigned account_login_result(const WireBytes& bytes) {
 wire_require(bytes.size()==5&&bytes[0]==0xC1&&bytes[1]==5&&bytes[2]==0xF1&&bytes[3]==1,"Invalid login response");
 return bytes[4]; // Preserve raw code; only 1 means success in JSProtocol.cpp.
}
}
