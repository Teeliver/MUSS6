#pragma once
#include "connect_protocol.hpp"
#include <set>
#include <string>
namespace mu::core {
struct LobbyCharacter {unsigned slot,level,control,guildStatus;std::string name;WireBytes charset;};
struct CharacterRoster {unsigned classCode,moveCount,extWarehouse;std::vector<LobbyCharacter> characters;};
struct CharacterPolicy {unsigned flag,classCode,experienceA,experienceB,maxLevel,deleteMaxLevel;};
inline unsigned character_u16(const WireBytes& b,unsigned offset){return unsigned(b.at(offset))|(unsigned(b.at(offset+1))<<8);}
inline WireBytes character_list_request(){return {0xC1,4,0xF3,0};}
inline CharacterRoster character_roster(const WireBytes& b){
 wire_require(b.size()>=8&&b[0]==0xC1&&b[1]==b.size()&&b[2]==0xF3&&b[3]==0,"Invalid character-list header");
 wire_require(b[6]<=5&&b.size()==8+unsigned(b[6])*34,"Invalid character-list count/size");
 CharacterRoster result{b[4],b[5],b[7],{}};std::set<unsigned> slots;std::set<std::string> names;
 for(unsigned i=0;i<b[6];i++){
  unsigned offset=8+i*34;std::string name;
  for(unsigned n=0;n<10&&b[offset+1+n]!=0;n++){
   auto c=b[offset+1+n];wire_require((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9'),"Unsupported character name encoding");name+=char(c);
  }
  wire_require(!name.empty()&&b[offset]<5&&slots.insert(b[offset]).second&&names.insert(name).second,"Invalid or duplicate character slot/name");
  result.characters.push_back({b[offset],character_u16(b,offset+12),b[offset+14],b[offset+33],name,WireBytes(b.begin()+offset+15,b.begin()+offset+33)});
 }
 return result;
}
inline CharacterPolicy character_policy(const WireBytes& b){
 // EXTRA1 server ABI has one padding byte at offset 5, then four WORD fields.
 wire_require(b.size()==14&&b[0]==0xC1&&b[1]==14&&b[2]==0xDE&&b[3]==0,"Invalid character creation policy");
 return {b[3],b[4],character_u16(b,6),character_u16(b,8),character_u16(b,10),character_u16(b,12)};
}
}
