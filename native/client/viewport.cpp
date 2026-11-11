#include "stdafx.h"
#include "Viewport.h"
#include "viewport.hpp"
#include <cstring>
#include <algorithm>
#include <set>
#include <stdexcept>
namespace mu::client { namespace {
static_assert(sizeof(PMSG_VIEWPORT_MONSTER)==10&&sizeof(PMSG_VIEWPORT_PLAYER)==36&&sizeof(PMSG_VIEWPORT_SUMMON)==20&&sizeof(PMSG_VIEWPORT_CHANGE)==38,"EX603 viewport layout changed");
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
unsigned id(const BYTE* p){return ((unsigned(p[0])<<8)|p[1])&0x3FFF;}
template<class T>T record(const std::vector<std::uint8_t>& b,std::size_t& offset){need(offset<=b.size()&&sizeof(T)<=b.size()-offset,"Truncated EX603 viewport record");T value{};std::memcpy(&value,b.data()+offset,sizeof(T));offset+=sizeof(T);return value;}
void base(Entity& e,const BYTE* index,unsigned x,unsigned y,unsigned tx,unsigned ty,unsigned dir){e.index=id(index);e.created=index[0]&0x80;e.teleport=index[0]&0x40;e.x=x;e.y=y;e.tx=tx;e.ty=ty;e.direction=dir>>4;e.pkLevel=dir&15;}
void effects(Entity& e,const std::vector<std::uint8_t>& b,std::size_t& offset,unsigned count){need(count<=b.size()-offset,"Truncated viewport effects");e.effects.assign(b.begin()+offset,b.begin()+offset+count);offset+=count;}
std::string name(const char* p){return std::string(p,std::find(p,p+10,'\0'));}
}
bool is_unarmored_dw(const Entity& e){const auto& a=e.appearance;return e.kind==EntityKind::Player&&(a[0]&0xF0)==0&&a[1]==255&&a[2]==255&&a[3]==255&&a[4]==255&&a[5]==0xF3&&a[9]==0xF8&&a[10]==0&&a[11]==0&&a[12]==0xF0&&a[13]==255&&a[14]==255&&a[15]==255&&a[16]==0&&a[17]==0;}
bool apply_viewport_packet(ViewportSnapshot& state,const std::vector<std::uint8_t>& b,unsigned self){
 need(b.size()>=3&&(b[0]==0xC1||b[0]==0xC2),"Viewport requires normalized C1/C2 packet");bool wide=b[0]==0xC2;unsigned off=wide?3:2;need(b.size()>off,"Truncated viewport header");need((wide?((unsigned(b[1])<<8)|b[2]):b[1])==b.size(),"Viewport frame size mismatch");unsigned head=b[off];
 if(head==0x20){need(wide&&b.size()>=sizeof(PMSG_VIEWPORT_SEND),"Invalid ground item header");std::size_t pos=sizeof(PMSG_VIEWPORT_SEND);std::map<unsigned,GroundItem> incoming;for(unsigned i=0;i<b[4];i++){auto p=record<PMSG_VIEWPORT_ITEM>(b,pos);GroundItem e;e.index=id(p.index);e.x=p.x;e.y=p.y;std::array<std::uint8_t,12> raw;std::copy_n(p.ItemInfo,12,raw.begin());e.item=decode_item(raw);if(e.item.index==14*512+15){e.money=(unsigned(raw[6])<<24)|(unsigned(raw[1])<<16)|(unsigned(raw[2])<<8)|raw[4];e.item.level=e.item.option=e.item.excellent=e.item.setOption=0;e.item.skill=e.item.luck=false;}need(incoming.emplace(e.index,e).second,"Duplicate ground item");}need(pos==b.size(),"Trailing ground item bytes");std::size_t added=0;for(auto& pair:incoming)added+=!state.items.count(pair.first);need(state.items.size()+added<=1024,"Ground item bound exceeded");if(!incoming.empty()){++state.revision;for(auto& pair:incoming){pair.second.revision=state.revision;state.items[pair.first]=pair.second;}}return true;}
 if(head==0x21){need(wide&&b.size()>=sizeof(PMSG_VIEWPORT_DESTROY_ITEM_SEND),"Invalid ground item destroy");std::size_t pos=sizeof(PMSG_VIEWPORT_DESTROY_ITEM_SEND);std::vector<unsigned> ids;for(unsigned i=0;i<b[4];i++){auto p=record<PMSG_VIEWPORT_DESTROY>(b,pos);ids.push_back(id(p.index));}need(pos==b.size(),"Trailing ground destroy bytes");bool changed=false;for(auto index:ids)changed|=state.items.erase(index)!=0;if(changed)++state.revision;return true;}
 if(head==0x12||head==0x13||head==0x1F||head==0x45){
  need(wide&&b.size()>=sizeof(PMSG_VIEWPORT_SEND),"Invalid viewport spawn header");std::size_t pos=sizeof(PMSG_VIEWPORT_SEND);unsigned count=b[4];std::vector<Entity> incoming;std::set<unsigned> seen;
  for(unsigned i=0;i<count;i++){Entity e;
   if(head==0x12){auto p=record<PMSG_VIEWPORT_PLAYER>(b,pos);base(e,p.index,p.x,p.y,p.tx,p.ty,p.DirAndPkLevel);e.kind=EntityKind::Player;e.name=name(p.name);std::copy_n(p.CharSet,18,e.appearance.begin());e.type=(e.appearance[0]>>5)&7;effects(e,b,pos,p.count);}
   else if(head==0x13){auto p=record<PMSG_VIEWPORT_MONSTER>(b,pos);base(e,p.index,p.x,p.y,p.tx,p.ty,p.DirAndPkLevel);e.type=((unsigned(p.type[0])<<8)|p.type[1])&0x0FFF;effects(e,b,pos,p.count);}
   else if(head==0x1F){auto p=record<PMSG_VIEWPORT_SUMMON>(b,pos);base(e,p.index,p.x,p.y,p.tx,p.ty,p.DirAndPkLevel);e.kind=EntityKind::Summon;e.name=name(p.name);e.type=((unsigned(p.type[0])<<8)|p.type[1])&0x0FFF;effects(e,b,pos,p.count);}
   else{auto p=record<PMSG_VIEWPORT_CHANGE>(b,pos);base(e,p.index,p.x,p.y,p.tx,p.ty,p.DirAndPkLevel);e.kind=EntityKind::ChangedPlayer;e.name=name(p.name);e.type=(unsigned(p.skin[0])<<8)|p.skin[1];std::copy_n(p.CharSet,18,e.appearance.begin());effects(e,b,pos,p.count);}
   need(seen.insert(e.index).second,"Duplicate viewport entity index");if(e.index!=self)incoming.push_back(std::move(e));
  }need(pos==b.size(),"Trailing viewport spawn bytes");std::size_t added=0;for(auto& e:incoming)added+=!state.entities.count(e.index);need(state.entities.size()+added<=1024,"Viewport entity bound exceeded");if(incoming.empty())return true;
  ++state.revision;for(auto& e:incoming){e.revision=state.revision;state.recentDead.erase(e.index);state.entities.insert_or_assign(e.index,std::move(e));}return true;
 }
 if(head==0x14){need(!wide&&b.size()>=sizeof(PMSG_VIEWPORT_DESTROY_SEND),"Invalid viewport destroy header");std::size_t pos=sizeof(PMSG_VIEWPORT_DESTROY_SEND);unsigned count=b[3];std::vector<unsigned> ids;for(unsigned i=0;i<count;i++){auto p=record<PMSG_VIEWPORT_DESTROY>(b,pos);ids.push_back(id(p.index));}need(pos==b.size(),"Trailing viewport destroy bytes");bool changed=false;for(auto index:ids){auto e=state.entities.find(index);if(e!=state.entities.end()&&e->second.dead){state.recentDead[index]=e->second;while(state.recentDead.size()>64){auto oldest=std::min_element(state.recentDead.begin(),state.recentDead.end(),[](const auto& a,const auto& b){return a.second.revision<b.second.revision;});state.recentDead.erase(oldest);}}changed|=state.entities.erase(index)!=0;}if(changed)++state.revision;return true;}
 if(head==PROTOCOL_CODE1||head==PROTOCOL_CODE3||head==0x17||head==0x18){
  need(!wide,"Invalid viewport update header");std::size_t pos=0;unsigned index=0,x=0,y=0,dir=0,action=0,target=0;
  if(head==PROTOCOL_CODE1){auto p=record<PMSG_MOVE_SEND>(b,pos);index=id(p.index);x=p.x;y=p.y;dir=p.dir>>4;}
  else if(head==PROTOCOL_CODE3){auto p=record<PMSG_POSITION_SEND>(b,pos);index=id(p.index);x=p.x;y=p.y;}
  else if(head==0x17){auto p=record<PMSG_USER_DIE_SEND>(b,pos);index=id(p.index);}
  else{auto p=record<PMSG_ACTION_SEND>(b,pos);index=id(p.index);dir=p.dir;action=p.action;target=id(p.target);need(dir<16,"Invalid viewport action direction");}
  need(pos==b.size(),"Invalid viewport update length");auto found=state.entities.find(index);if(found==state.entities.end()||index==self)return true;auto& e=found->second;
  e.teleport=false;e.created=false;if(head==0x17)e.dead=true;else if(head==0x18){e.direction=dir;e.action=action;e.target=target;e.actionRevision=state.revision+1;}else{e.tx=x;e.ty=y;if(head==PROTOCOL_CODE1)e.direction=dir;if(head==PROTOCOL_CODE3){e.x=x;e.y=y;e.teleport=true;}}
  e.revision=++state.revision;return true;
 }return false;
}
}
