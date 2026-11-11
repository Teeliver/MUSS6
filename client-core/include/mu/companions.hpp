#pragma once
#include "connect_protocol.hpp"
namespace mu::core {
// status: 0 none, 1 builtin item, 2 custom catalog required, 3 unknown encoding, 4 builtin/custom ambiguity.
struct Companion {unsigned status,index,variant,customIndex;};
struct Companions {Companion wing,pet;WireBytes raw;};
inline Companions decode_companions(const WireBytes& a){
 wire_require(a.size()==18,"Companions require 18 bytes");
 Companions result{{3,0,0,0},{3,0,0,0},a};auto& w=result.wing;auto& p=result.pet;
 unsigned family=a[5]&12,code=a[9]&7,high=a[17]>>5,custom=(a[17]>>1)&15,extra=(a[16]>>2)&7;
 if(family==0&&code==0&&high==0&&custom==0&&extra==0)w={0,0,0,0};
 else if(family==12&&(a[17]&254)&&code==0&&extra==0){
  if(high>=1&&high<=6&&custom==0)w={4,12*512+129+high,0,(unsigned(a[17])>>1)-1};
  else w={2,0,0,(unsigned(a[17])>>1)-1};
 }
 else if(family==8&&extra>=1&&extra<=4&&code==0&&custom==0&&high==0)w={1,12*512+261+extra,0,0};
 else if(high==0&&custom==0&&extra==0){
  unsigned type=511;
  if(family==4){if(code>=1&&code<=3)type=code-1;else if(code==4)type=41;else if(code==5)type=266;else if(code==6)type=267;}
  if(family==8){if(code>=1&&code<=4)type=code+2;else if(code==5)type=30;else if(code==6)type=42;else if(code==7)type=49;}
  if(family==12){if(code>=1&&code<=5)type=code+35;else if(code==6)type=43;else if(code==7)type=50;}
  if(type!=511)w={1,12*512+type,0,0};
 }
 unsigned extended=a[16]&224,base=a[5]&3;
 if(extended){
  unsigned type=511;
  switch(extended){case 32:type=64;break;case 64:type=65;break;case 128:type=67;break;case 224:type=80;break;case 160:type=106;break;case 96:type=123;break;}
  if(type!=511)p={1,13*512+type,0,0};
 }else if(base<3)p={1,13*512+base,0,0};
 else if(a[12]&4)p={1,13*512+37,(a[16]&1)?1u:(a[16]&2)?2u:(a[17]&1)?4u:0u,0};
 else if(a[12]&1)p={1,13*512+4,0,0};
 else if(a[10]&1)p={1,13*512+3,0,0};
 else p={0,0,0,0};
 return result;
}
}
