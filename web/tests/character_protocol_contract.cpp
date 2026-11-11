#include "mu/character_protocol.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_CHARACTER_REFERENCE
#include "character_reference.hpp"
#include <cstddef>
#include <cstring>
static_assert(sizeof(character_reference::PMSG_CHARACTER_LIST_SEND)==8);
static_assert(sizeof(character_reference::PMSG_CHARACTER_LIST)==34);
static_assert(offsetof(character_reference::PMSG_CHARACTER_LIST,Level)==12);
static_assert(offsetof(character_reference::PMSG_CHARACTER_LIST,CharSet)==15);
static_assert(sizeof(character_reference::PMSG_CHARACTER_CREATION_ENABLE_SEND)==14);
static_assert(offsetof(character_reference::PMSG_CHARACTER_CREATION_ENABLE_SEND,CharacterDeleteMaxLevel)==12);
#endif
using namespace mu::core;
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}assert(failed);}
int main(){
 assert(character_list_request()==WireBytes({0xC1,4,0xF3,0}));
 for(unsigned count=0;count<=5;count++){
  WireBytes b(8+count*34,0);b[0]=0xC1;b[1]=std::uint8_t(b.size());b[2]=0xF3;b[4]=7;b[5]=2;b[6]=count;b[7]=3;
  for(unsigned i=0;i<count;i++){
   auto off=8+i*34;b[off]=i;b[off+1]='A'+i;b[off+11]=0xA5;b[off+12]=0x34;b[off+13]=0x12;b[off+14]=18;b[off+33]=255;
   for(unsigned n=0;n<18;n++)b[off+15+n]=std::uint8_t(i*32+n);
#ifdef MU_CHARACTER_REFERENCE
   character_reference::PMSG_CHARACTER_LIST row{};std::memcpy(&row,b.data()+off,sizeof(row));assert(row.slot==i&&row.Level==0x1234&&row.CtlCode==18&&row.CharSet[17]==i*32+17&&row.GuildStatus==255);
#endif
  }
  auto roster=character_roster(b);assert(roster.characters.size()==count&&roster.moveCount==2&&roster.extWarehouse==3);
  for(unsigned i=0;i<count;i++)assert(roster.characters[i].name==std::string(1,char('A'+i))&&roster.characters[i].level==0x1234&&roster.characters[i].charset[17]==i*32+17);
  for(unsigned n=0;n<b.size();n++){auto shortPacket=WireBytes(b.begin(),b.begin()+n);rejects([&]{character_roster(shortPacket);});}
  if(count>1){auto duplicate=b;duplicate[42]=0;rejects([&]{character_roster(duplicate);});duplicate=b;duplicate[43]='A';rejects([&]{character_roster(duplicate);});}
 }
 WireBytes p{0xC1,14,0xDE,0,7,0xFF,1,2,3,4,5,6,7,8};auto policy=character_policy(p);assert(policy.deleteMaxLevel==0x0807&&policy.maxLevel==0x0605&&policy.experienceA==0x0201);
#ifdef MU_CHARACTER_REFERENCE
 character_reference::PMSG_CHARACTER_CREATION_ENABLE_SEND ref{};std::memcpy(&ref,p.data(),sizeof(ref));assert(ref.CharacterDeleteMaxLevel==policy.deleteMaxLevel&&ref.CharacterMaxLevel==policy.maxLevel);
#endif
 p[3]=1;rejects([&]{character_policy(p);});
 std::cout<<"character protocol contract passed\n";
}
