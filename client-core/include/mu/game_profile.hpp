#pragma once
#include "connect_protocol.hpp"
#include <array>
namespace mu::core {
// This server build fixes CustomerName to MuOnline in InitHackCheck.
inline TransportKeys derive_transport_keys(const std::string& serial,unsigned raw1,unsigned raw2) {
 wire_require(serial.size()==16&&raw1<=255&&raw2<=255,"Invalid transport profile");
 for(unsigned char c:serial)wire_require(c>=33&&c<=126,"Invalid serial byte encoding");
 const std::string customer="MuOnline";std::uint16_t key=0,sum=0;
 for(unsigned n=0;n<32;n++){
  auto c=std::uint8_t(n<customer.size()?customer[n]:0),s=std::uint8_t(n%17<16?serial[n%17]:0);
  key=std::uint16_t(key+std::uint8_t(c^s));key^=std::uint8_t(c-s);sum=std::uint16_t(sum+c);
 }
 const bool mhp=raw1!=0||raw2!=0;
 return {std::uint8_t(0xAB+(key&255)),std::uint8_t(0xCD+(key>>8)),mhp,
  std::uint8_t(mhp?raw1+(sum&255):0),std::uint8_t(mhp?raw2+(sum>>8):0)};
}
struct GameGreeting {unsigned index,serverCode;std::array<std::uint8_t,5> encodedVersion;};
inline GameGreeting game_greeting(const WireBytes& b) {
 wire_require(b.size()==14&&b[0]==0xC1&&b[1]==14&&b[2]==0xF1&&b[3]==0,"Invalid EX603 EXTRA1 greeting");
 wire_require(b[4]==1,"GameServer rejected connection");
 GameGreeting result{(unsigned(b[5])<<8)|b[6],unsigned(b[12])|(unsigned(b[13])<<8),{}};
 std::copy_n(b.begin()+7,5,result.encodedVersion.begin());return result;
}
}
