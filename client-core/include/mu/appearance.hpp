#pragma once
#include "connect_protocol.hpp"
#include <array>
namespace mu::core {
struct AppearancePart {unsigned slot,index,visualLevel;bool present,excellent,ancient;};
struct Appearance {unsigned baseClass,tier;std::array<AppearancePart,7> parts;WireBytes raw;};
inline Appearance decode_appearance(const WireBytes& a){
 wire_require(a.size()==18,"Appearance requires 18 bytes");unsigned base=a[0]>>5;
 wire_require(base<=6,"Unsupported appearance class");
 const unsigned indices[]={unsigned(a[1])|((unsigned(a[12])&240)<<4),unsigned(a[2])|((unsigned(a[13])&240)<<4),
 unsigned(a[3]>>4)|((a[9]&128)>>3)|((a[13]&15)<<5),unsigned(a[3]&15)|((a[9]&64)>>2)|((a[14]&240)<<1),
 unsigned(a[4]>>4)|((a[9]&32)>>1)|((a[14]&15)<<5),unsigned(a[4]&15)|(a[9]&16)|((a[15]&240)<<1),
 unsigned(a[5]>>4)|((a[9]&8)<<1)|((a[15]&15)<<5)};
 const unsigned flags[]={4,2,128,64,32,16,8},grades[]={0,3,5,7,9,11,13,15};
 unsigned packed=(unsigned(a[6])<<16)|(unsigned(a[7])<<8)|a[8];
 Appearance result{base,(a[0]&28)>=28?2u:(a[0]&16)?1u:0u,{},a};
 for(unsigned i=0;i<7;i++){
  bool present=indices[i]!=(i<2?4095u:511u);
  result.parts[i]={i,i<2?indices[i]:(i+5)*512+indices[i],present?grades[(packed>>(i*3))&7]:0,
   present,present&&bool(a[10]&flags[i]),present&&bool(a[11]&flags[i])};
 }
 return result;
}
}
