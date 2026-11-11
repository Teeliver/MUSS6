#include "mu/appearance.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_APPEARANCE_REFERENCE
#include "appearance_reference.hpp"
#endif
using namespace mu::core;
int main(){
#ifdef MU_APPEARANCE_REFERENCE
 // Compare every 9-bit armor id, each hand's 12-bit id, all levels and flags against source packing.
 for(unsigned slot=0;slot<7;slot++)for(unsigned id=0;id<(slot<2?4096u:512u);id++){
  int inventory[]={0xFFFF,0xFFFF,511,511,511,511,511};inventory[slot]=int(id);
  appearance_reference::Input input;input.Inventory[1+slot*5]=15*8;input.Inventory[2+slot*5]=1;input.Inventory[3+slot*5]=1;
  auto packed=appearance_reference::pack(inventory,&input);auto a=decode_appearance(WireBytes(packed.CharSet,packed.CharSet+18));
  const auto& part=a.parts[slot];bool present=id!=(slot<2?4095u:511u);assert(part.present==present);
  assert(part.index==(slot<2?id:(slot+5)*512+id));if(present){if(id==511)assert(part.visualLevel==0&&!part.excellent&&!part.ancient);else assert(part.visualLevel==15&&part.excellent&&part.ancient);}
 }
 for(unsigned level=0;level<=15;level++){
  int ids[]={0,0,0,0,0,0,0};appearance_reference::Input input;
  for(unsigned slot=0;slot<7;slot++)input.Inventory[1+slot*5]=level*8;
  auto packed=appearance_reference::pack(ids,&input);auto a=decode_appearance(WireBytes(packed.CharSet,packed.CharSet+18));
  unsigned expected=level<3?0:(level%2?level:level-1);
  for(const auto& p:a.parts)assert(p.visualLevel==expected&&!p.excellent&&!p.ancient);
 }
#endif
 for(unsigned base=0;base<7;base++)for(unsigned tier=0;tier<3;tier++){
  WireBytes bytes(18);bytes[0]=std::uint8_t((base<<5)|(tier==2?28:tier==1?16:0));auto a=decode_appearance(bytes);assert(a.baseClass==base&&a.tier==tier);bytes[0]=0;assert(a.raw[0]==((base<<5)|(tier==2?28:tier==1?16:0)));
 }
 bool failed=false;try{decode_appearance(WireBytes(17));}catch(...){failed=true;}assert(failed);
 failed=false;try{WireBytes b(18);b[0]=224;decode_appearance(b);}catch(...){failed=true;}assert(failed);
 std::cout<<"appearance contract passed\n";
}
