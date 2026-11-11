#include "ground_item_style.hpp"
#include "ground_controls.hpp"
#include "stdafx.h"
#include "ServerInfo.h"
#include "HackCheck.h"
#include "client.hpp"
#include "Protocol.h"
#include "Viewport.h"
#include "SkillManager.h"
#include "ItemManager.h"
#include "evil_spirit.hpp"
#include <filesystem>
#include <fstream>
#include <cassert>
// Only the original transport routines are linked, not gameplay or server startup.
CServerInfo gServerInfo;
CServerInfo::CServerInfo(){}
CServerInfo::~CServerInfo(){}
template<class F> void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}assert(failed);}
int main(){
 {using namespace mu::client;ViewportSnapshot v;PMSG_VIEWPORT_ITEM item{};item.index[1]=7;item.x=148;item.y=120;item.ItemInfo[0]=3;item.ItemInfo[1]=9*8;item.ItemInfo[5]=0x50;Bytes spawn={0xC2,0,static_cast<std::uint8_t>(5+sizeof(item)),0x20,1};auto raw=reinterpret_cast<const std::uint8_t*>(&item);spawn.insert(spawn.end(),raw,raw+sizeof(item));assert(apply_viewport_packet(v,spawn,42)&&v.items.at(7).item.index==2563&&v.items.at(7).item.level==9);auto rev=v.revision;auto bad=spawn;bad.pop_back();rejects([&]{apply_viewport_packet(v,bad,42);});assert(v.revision==rev&&v.items.size()==1);assert(apply_viewport_packet(v,{0xC2,0,7,0x21,1,0,7},42)&&v.items.empty());InventorySnapshot inv;inv.ready=true;PMSG_ITEM_GET_SEND pickup{};pickup.header.set(0x22,sizeof(pickup));pickup.result=12;std::copy_n(item.ItemInfo,12,pickup.ItemInfo);auto start=reinterpret_cast<const std::uint8_t*>(&pickup);assert(apply_inventory_packet(inv,Bytes(start,start+sizeof(pickup)))&&inv.items.at(12).level==9);assert(apply_inventory_packet(inv,{0xC1,5,0x23,0,12})&&inv.items.count(12));assert(apply_inventory_packet(inv,{0xC1,5,0x23,1,12})&&!inv.items.count(12));PMSG_DURATION_SKILL_ATTACK_SEND cast{};cast.header.set(0x1E,sizeof(cast));cast.index[1]=42;cast.skill[1]=9;CombatState combat;start=reinterpret_cast<const std::uint8_t*>(&cast);assert(apply_combat_packet(combat,Bytes(start,start+sizeof(cast)),42)&&combat.skill==9&&combat.castRevision==1&&combat.castTarget==0);EvilSpirits a(100,100,7),b(100,100,7);for(unsigned i=0;i<50;i++)a.step(100,100);for(unsigned i=0;i<100;i++)b.advance(.02,100,100);assert(a.frame==50&&b.frame==50);for(unsigned i=0;i<4;i++){assert(a.heads[i].tail.size()==6);assert(std::abs(a.heads[i].p.x-b.heads[i].p.x)<1e-9&&std::abs(a.heads[i].p.y-b.heads[i].p.y)<1e-9);}}
 {using namespace mu::client;EquipmentAttributes a;auto wire=[](const auto& p){auto data=reinterpret_cast<const std::uint8_t*>(&p);return Bytes(data,data+sizeof(p));};PMSG_CHARACTER_INFO_SEND p{};p.header.setE(0xF3,3,sizeof(p));p.ViewPoint=321;p.ViewStrength=70000;p.ViewDexterity=80;p.ViewEnergy=90;p.ViewVitality=100;p.ViewLeadership=110;p.Class=2;assert(apply_character_attributes(a,wire(p))&&a.ready&&a.base[0]==70000&&a.base[2]==90&&a.base[3]==100&&a.baseClass==2&&a.points==321);PMSG_NEW_CHARACTER_CALC_SEND bonus{};bonus.header.set(0xF3,0xE1,sizeof(bonus));bonus.ViewPhysiDamageMin=123;bonus.ViewPhysiDamageMax=456;bonus.ViewDefense=789;bonus.ViewAddStrength=7;bonus.ViewAddVitality=9;assert(apply_character_attributes(a,wire(bonus))&&a.bonus[0]==7&&a.bonus[3]==9&&a.base[0]==70000);assert(a.combatReady&&a.combat[0]==123&&a.combat[1]==456&&a.combat[4]==789);PMSG_NEW_CHARACTER_INFO_SEND stats{};stats.header.set(0xF3,0xE0,sizeof(stats));stats.Level=400;stats.ViewStrength=80000;assert(apply_character_attributes(a,wire(stats))&&a.level==400&&a.base[0]==80000&&a.bonus[0]==7);auto bad=wire(stats);bad.pop_back();rejects([&]{apply_character_attributes(a,bad);});assert(!apply_character_attributes(a,{0xC2,0,0xF3,3}));}

 {mu::client::Bytes notice={0xC1,17,0x0D,1,0,0,0,0,0,0,0,0,0,'5','s','!',0};auto n=mu::client::parse_notice(notice);assert(n&&n->type==1&&n->message=="5s!");notice.pop_back();notice[1]=notice.size();rejects([&]{mu::client::parse_notice(notice);});notice.resize(8);notice[1]=8;rejects([&]{mu::client::parse_notice(notice);});assert(!mu::client::parse_notice({0xC1,3,0x02}));}

 {using namespace mu::client;InventorySnapshot inv;inv.ready=true;Bytes reply(16,0);reply[0]=0xC1;reply[1]=16;reply[2]=0x32;reply[3]=12;reply[4]=3;reply[5]=9*8;reply[6]=56;reply[9]=0x70;assert(apply_inventory_packet(inv,reply)&&inv.items.at(12).index==7*512+3&&inv.items.at(12).level==9);auto before=inv.revision;reply[3]=255;assert(apply_inventory_packet(inv,reply)&&inv.revision==before);reply[3]=12;rejects([&]{apply_inventory_packet(inv,reply);});reply[3]=2;rejects([&]{apply_inventory_packet(inv,reply);});}
 {mu::client::InventorySnapshot inv;inv.ready=true;auto revision=inv.revision;assert(mu::client::apply_inventory_packet(inv,{0xC1,5,0x28,0xFF,1})&&inv.revision==revision);}
 {mu::client::CombatState combat;PMSG_SKILL_ATTACK_SEND cast{};cast.header.set(0x19,sizeof(cast));cast.index[1]=42;cast.skill[1]=17;cast.target[0]=0x80;cast.target[1]=3;auto bytes=[](const auto& p){auto begin=reinterpret_cast<const std::uint8_t*>(&p);return mu::client::Bytes(begin,begin+sizeof(p));};assert(mu::client::apply_combat_packet(combat,bytes(cast),42));assert(combat.castTarget==3&&combat.castRevision==1);PMSG_DAMAGE_SEND hit{};hit.header.set(PROTOCOL_CODE2,sizeof(hit));hit.index[1]=99;hit.ViewDamageHP=123;mu::client::apply_combat_packet(combat,bytes(hit),42);assert(combat.castTarget==3&&combat.target==99&&combat.damage==123&&combat.hits.size()==1);PMSG_REWARD_EXPERIENCE_SEND kill{};kill.header.set(0x9C,sizeof(kill));kill.index[0]=0x80;kill.index[1]=3;kill.ViewDamageHP=321;mu::client::apply_combat_packet(combat,bytes(kill),42);assert(combat.castTarget==3&&combat.target==3&&combat.damage==321&&combat.hits.size()==2&&combat.kills==1);rejects([&]{mu::client::apply_combat_packet(combat,{0xC1,3,PROTOCOL_CODE2},42);});}
 assert((mu::client::normal_attack_packet(0x1234,7)==mu::client::Bytes{0xC1,7,PROTOCOL_CODE2,0x12,0x34,120,7}));rejects([]{mu::client::normal_attack_packet(0x4000,0);});rejects([]{mu::client::normal_attack_packet(1,8);});
 assert((mu::client::skill_attack_packet(17,0x1234)==mu::client::Bytes{0xC3,8,0x19,0,17,0x12,0x34,0}));rejects([]{mu::client::skill_attack_packet(999,1);});
 for(unsigned type=0;type<3;type++)assert((mu::client::logout_packet(type)==mu::client::Bytes{0xC3,5,0xF1,2,std::uint8_t(type)}));
 bool badLogout=false;try{mu::client::logout_packet(3);}catch(const std::exception&){badLogout=true;}assert(badLogout);
 {using namespace mu::client;std::array<std::uint8_t,12> raw{};raw[0]=1;raw[1]=0xFC;raw[2]=40;raw[3]=32;Item item=decode_item(raw);InventorySnapshot inv;inv.ready=true;inv.items[14]=item;ItemSlots vault;ItemMove move{0,14,2,0};auto packet=item_move_packet(move,item);assert(packet.size()==19&&packet[0]==0xC3&&packet[2]==0x24&&packet[3]==0&&packet[4]==14&&packet[17]==2&&packet[18]==0);assert(std::equal(raw.begin(),raw.end(),packet.begin()+5));std::vector<std::uint8_t> reply={0xC1,17,0x24,255,0};reply.insert(reply.end(),12,255);assert(!commit_item_move(inv,vault,move,item,reply));assert(inv.items.count(14)&&vault.empty()&&inv.revision==0);reply[3]=2;std::copy(raw.begin(),raw.end(),reply.begin()+5);auto malformed=reply;malformed[4]=1;rejects([&]{commit_item_move(inv,vault,move,item,malformed);});assert(inv.items.count(14)&&vault.empty());assert(commit_item_move(inv,vault,move,item,reply));assert(inv.items.empty()&&vault.at(0).wire==raw&&inv.revision==1);ItemMove back{2,0,0,15};reply[3]=0;reply[4]=15;assert(commit_item_move(inv,vault,back,item,reply));assert(vault.empty()&&inv.items.at(15).wire==raw&&inv.revision==2);auto equippedPacket=item_move_packet({0,15,0,0},item);assert(equippedPacket[4]==15&&equippedPacket[18]==0);reply[4]=0;assert(commit_item_move(inv,vault,{0,15,0,0},item,reply));assert(inv.items.count(0)&&!inv.items.count(15));reply[4]=15;assert(commit_item_move(inv,vault,{0,0,0,15},item,reply));assert(inv.items.count(15)&&!inv.items.count(0));rejects([&]{item_move_packet({0,76,2,0},item);});rejects([&]{item_move_packet({0,15,2,120},item);});}

 using namespace mu::client;
 auto deletion=delete_character_packet("TestChar","1234567");assert(deletion.size()==sizeof(PMSG_CHARACTER_DELETE_RECV)&&deletion[0]==0xC1&&deletion[2]==0xF3&&deletion[3]==2);assert(std::string(reinterpret_cast<const char*>(deletion.data()+4),8)=="TestChar");assert(deletion[12]==0&&deletion[13]==0&&deletion[21]==0&&deletion[22]==0&&deletion[23]==0);assert(std::string(reinterpret_cast<const char*>(deletion.data()+14),7)=="1234567");rejects([]{delete_character_packet("TestChar","notdigits");});rejects([]{delete_character_packet("TestChar","12345678901");});
 auto servers=parse_list({0xC2,0,11,0xF4,6,0,1,0,0,34,0xCC});
 assert(servers.size()==1&&servers[0].code==0&&servers[0].load==34);
 rejects([]{parse_list({0xC2,0,11,0xF4,6,0,2,0,0,34,0xCC});});
 rejects([]{parse_list({0xC2,0,15,0xF4,6,0,2,0,0,0,0xCC,0,0,0,0xCC});});
 Bytes inventory={0xC2,0,19,0xF3,0x10,1,6,0,40,255,0,0,0xB0,0,255,255,255,255,255};assert(parse_boots_level(inventory)==5);inventory[8]=32;assert(parse_boots_level(inventory)==4);inventory[12]=0xA0;rejects([&]{parse_boots_level(inventory);});assert(parse_boots_level({0xC2,0,6,0xF3,0x10,0})==-1);rejects([]{parse_boots_level({0xC2,0,6,0xF3,0x10,1});});
 auto movement=move_packet(135,124,3);assert(movement.size()==13&&movement[2]==0xD4&&movement[3]==135&&movement[4]==124&&movement[5]==0x31&&movement[6]==0x30);
 auto packed=path_packet(135,124,{3,4,5,6,7,0,1,2,3,4,5,6,7,0});assert(packed[5]==0x3E&&packed[6]==0x34&&packed[7]==0x56&&packed[12]==0x70);
 rejects([]{path_packet(135,124,std::vector<unsigned>(15,3));});rejects([]{path_packet(0,0,{7});});rejects([]{path_packet(135,124,{});});
 rejects([]{move_packet(0,0,8);});rejects([]{move_packet(256,0,1);});
 ViewportSnapshot viewport;PMSG_VIEWPORT_SEND spawnHead{};spawnHead.header.set(0x13,28);spawnHead.count=2;PMSG_VIEWPORT_MONSTER npc{};npc.index[0]=0x81;npc.index[1]=0x41;npc.type[1]=240;npc.x=npc.tx=147;npc.y=npc.ty=145;npc.DirAndPkLevel=0x13;npc.count=2;PMSG_VIEWPORT_MONSTER wizard=npc;wizard.index[1]=0x42;wizard.type[1]=254;wizard.count=1;
 Bytes spawn(sizeof(spawnHead));std::memcpy(spawn.data(),&spawnHead,sizeof(spawnHead));auto append=[&](const auto& value){auto ptr=reinterpret_cast<const std::uint8_t*>(&value);spawn.insert(spawn.end(),ptr,ptr+sizeof(value));};append(npc);spawn.push_back(7);spawn.push_back(8);append(wizard);spawn.push_back(9);
 assert(apply_viewport_packet(viewport,spawn,11900)&&viewport.entities.size()==2&&viewport.entities.at(321).type==240&&viewport.entities.at(321).created&&viewport.entities.at(321).effects.size()==2&&viewport.entities.at(322).type==254);
 auto originalRevision=viewport.revision;auto badSpawn=spawn;badSpawn.pop_back();badSpawn[2]--;rejects([&]{apply_viewport_packet(viewport,badSpawn,11900);});assert(viewport.revision==originalRevision&&viewport.entities.size()==2);
 auto duplicate=spawn;duplicate[18]=0x41;rejects([&]{apply_viewport_packet(viewport,duplicate,11900);});assert(viewport.revision==originalRevision);
 assert(apply_viewport_packet(viewport,{0xC1,8,0xD4,1,0x41,148,146,0x30},11900)&&viewport.entities.at(321).tx==148&&viewport.entities.at(321).direction==3);
 assert(apply_viewport_packet(viewport,{0xC1,7,0x15,1,0x41,147,145},11900)&&viewport.entities.at(321).teleport&&viewport.entities.at(321).direction==3);
 assert(apply_viewport_packet(viewport,{0xC1,4,0x14,0},11900));auto beforeUnknown=viewport.revision;assert(apply_viewport_packet(viewport,{0xC1,8,0xD4,1,0x7F,120,120,0x10},11900)&&viewport.revision==beforeUnknown);
 assert(apply_viewport_packet(viewport,{0xC1,6,0x14,1,1,0x41},11900)&&viewport.entities.size()==1&&!viewport.entities.count(321));
 ViewportSnapshot filtered;assert(apply_viewport_packet(filtered,spawn,321)&&filtered.entities.size()==1);rejects([&]{apply_viewport_packet(filtered,{0xC1,6,0x14,2,1,0x42},321);});assert(filtered.entities.size()==1);
 PMSG_USER_DIE_SEND dead{};dead.header.set(0x17,sizeof(dead));dead.index[0]=1;dead.index[1]=0x42;Bytes deathBytes(sizeof(dead));std::memcpy(deathBytes.data(),&dead,sizeof(dead));assert(apply_viewport_packet(filtered,deathBytes,321)&&filtered.entities.at(322).dead);assert(apply_viewport_packet(filtered,{0xC1,6,0x14,1,1,0x42},321)&&filtered.entities.empty()&&filtered.recentDead.at(322).dead);assert(apply_viewport_packet(filtered,spawn,321)&&filtered.recentDead.empty());
 PMSG_ACTION_SEND action{};action.header.set(0x18,sizeof(action));action.index[0]=1;action.index[1]=0x42;action.action=120;action.dir=3;action.target[0]=46;action.target[1]=124;Bytes actionWire(sizeof(action));std::memcpy(actionWire.data(),&action,sizeof(action));assert(apply_viewport_packet(filtered,actionWire,11900));auto attackRevision=filtered.entities.at(322).actionRevision;assert(attackRevision&&filtered.entities.at(322).action==120&&filtered.entities.at(322).target==11900);assert(apply_viewport_packet(filtered,{0xC1,8,0xD4,1,0x42,122,123,0x30},11900));assert(filtered.entities.at(322).actionRevision==attackRevision);
 mu::input::GroundNames labels;labels.flags(true);labels.lock();labels.flags(true);assert(labels.latched);labels.flags(false);assert(labels.latched);labels.flags(true);assert(!labels.latched);labels.flags(false);labels.lock();labels.flags(false);assert(labels.latched);
 labels=mu::input::GroundNames{};labels.poll(true,false);labels.poll(true,true);labels.poll(false,false);assert(labels.latched);labels.poll(true,false);assert(!labels.latched);mu::client::Item groundStyle;groundStyle.index=7*512;assert((mu::ui::ground_item_style(groundStyle).text==std::array<unsigned,3>{178,178,178}));groundStyle.level=7;assert((mu::ui::ground_item_style(groundStyle).text==std::array<unsigned,3>{255,204,25}));groundStyle.excellent=1;assert((mu::ui::ground_item_style(groundStyle).text==std::array<unsigned,3>{25,255,127}));groundStyle.setOption=5;assert((mu::ui::ground_item_style(groundStyle).background==std::array<unsigned,3>{60,60,200}));
 Entity naked;naked.kind=EntityKind::Player;naked.appearance={0,255,255,255,255,0xF3,0,0,0,0xF8,0,0,0xF0,255,255,255,0,0};assert(is_unarmored_dw(naked));naked.appearance[1]=0;assert(!is_unarmored_dw(naked));naked.appearance[1]=255;naked.appearance[0]=0x20;assert(!is_unarmored_dw(naked));
 PMSG_VIEWPORT_PLAYER peer{};peer.index[0]=1;peer.index[1]=0x43;peer.CharSet[0]=0x20;peer.DirAndPkLevel=0x10;std::memcpy(peer.name,"OtherDK",7);PMSG_VIEWPORT_SEND peerHead{};peerHead.header.set(0x12,sizeof(peerHead)+sizeof(peer));peerHead.count=1;Bytes peerWire(sizeof(peerHead)+sizeof(peer));std::memcpy(peerWire.data(),&peerHead,sizeof(peerHead));std::memcpy(peerWire.data()+sizeof(peerHead),&peer,sizeof(peer));assert(apply_viewport_packet(viewport,peerWire,11900)&&viewport.entities.at(323).type==1&&viewport.entities.at(323).name=="OtherDK");
 assert((npc_talk_packet(321)==Bytes{0xC3,5,0x30,1,65}));rejects([]{npc_talk_packet(0x4000);});
 InventorySnapshot bag;Bytes initial={0xC2,0,19,0xF3,0x10,1,6,0,40,255,0,0,0xB0,0,255,255,255,255,255};assert(apply_inventory_packet(bag,initial)&&bag.ready&&bag.items.at(6).index==5632&&bag.items.at(6).level==5);auto revision=bag.revision;auto broken=initial;broken.pop_back();rejects([&]{apply_inventory_packet(bag,broken);});assert(bag.revision==revision&&bag.items.size()==1);assert(apply_inventory_packet(bag,{0xC1,6,0x2A,6,200,0})&&bag.items.at(6).durability==200&&bag.items.at(6).wire[2]==200);assert(apply_inventory_packet(bag,{0xC1,5,0x28,6,1})&&bag.items.empty());std::array<std::uint8_t,12> optionItem{};optionItem[0]=7;optionItem[1]=128|4|3|(15<<3);optionItem[3]=128|64|33;optionItem[4]=5;optionItem[5]=0x28;auto option=decode_item(optionItem);assert(option.index==1287&&option.level==15&&option.skill&&option.luck&&option.option==7&&option.excellent==33&&option.setOption==5&&option.extended);
 Profile loginProfile;loginProfile.serial="1234567890123456";loginProfile.version="10200";
 auto login=login_packet(loginProfile,"Verify","Abc123",0x12345678);
 assert(login.size()==sizeof(PMSG_CONNECT_ACCOUNT_RECV)&&login[0]==0xC3&&login[2]==0xF1&&login[3]==1);
 PMSG_CONNECT_ACCOUNT_RECV loginStruct{};std::memcpy(&loginStruct,login.data(),login.size());
 assert(loginStruct.TickCount==0x12345678&&std::memcmp(loginStruct.ClientSerial,loginProfile.serial.data(),16)==0);
 const unsigned char credentialXor[3]={0xFC,0xCF,0xAB};
 for(unsigned i=0;i<10;i++){
  assert((static_cast<unsigned char>(loginStruct.account[i])^credentialXor[i%3])==(i<6?"Verify"[i]:0));
  assert((static_cast<unsigned char>(loginStruct.password[i])^credentialXor[i%3])==(i<6?"Abc123"[i]:0));
 }
 rejects([&]{login_packet(loginProfile,"bad'name","secret",0);});
 rejects([&]{login_packet(loginProfile,"Verify","12345678901",0);});
 PMSG_CHARACTER_LIST_SEND listHead{};listHead.header.set(0xF3,0,sizeof(listHead)+sizeof(PMSG_CHARACTER_LIST));listHead.count=1;
 PMSG_CHARACTER_LIST listRow{};listRow.slot=2;listRow.Level=257;std::memcpy(listRow.Name,"VerifyDW",8);
 Bytes listBytes(sizeof(listHead)+sizeof(listRow));std::memcpy(listBytes.data(),&listHead,sizeof(listHead));std::memcpy(listBytes.data()+sizeof(listHead),&listRow,sizeof(listRow));
 auto characters=parse_characters(listBytes);assert(characters.characters.size()==1&&characters.characters[0].level==257&&characters.characters[0].slot==2&&characters.characters[0].name=="VerifyDW");
 auto shortList=listBytes;shortList.pop_back();rejects([&]{parse_characters(shortList);});
 auto badSlot=listBytes;badSlot[sizeof(listHead)]=5;rejects([&]{parse_characters(badSlot);});
 Bytes greeting={0xC1,14,0xF1,0,1,0x2E,0x7C,49,48,50,48,48,0,0};
 assert(parse_greeting(greeting).index==11900);
 auto rejected=greeting;rejected[4]=0;rejects([&]{parse_greeting(rejected);});
 auto truncated=greeting;truncated.pop_back();rejects([&]{parse_greeting(truncated);});
 const auto root=std::filesystem::temp_directory_path()/("mu-client-contract-"+std::to_string(getpid()));
 std::filesystem::create_directories(root/"game/Data");
 try{
  for(bool mhp:{false,true}){
   int a=mhp?23:0,b=mhp?45:0;
   std::ofstream(root/"profile.ini")<<"[Client]\nConnectHost=127.0.0.1\nConnectPort=44405\nGameRuntime="<<(root/"game").string()<<"\n";
   std::ofstream(root/"game/Data/GameServerInfo - Common.dat")<<"[GameServerInfo]\nServerSerial=1234567890123456\nServerEncDecKey1="<<a<<"\nServerEncDecKey2="<<b<<"\n";
   auto p=load_profile((root/"profile.ini").string());
   std::memcpy(gServerInfo.m_ServerSerial,"1234567890123456",17);
   gServerInfo.m_ServerEncDecKey1=a;gServerInfo.m_ServerEncDecKey2=b;InitHackCheck();
   auto wire=greeting;EncryptData(wire.data(),static_cast<int>(wire.size()));
   // Reference encryption is the unchanged GameServer/HackCheck.cpp implementation.
   for(auto& byte:wire){Bytes chunk{byte};decode_transport(chunk,p);byte=chunk[0];}
   assert(wire==greeting);assert(parse_greeting(wire).index==11900);
   auto outbound=greeting;encode_transport(outbound,p);DecryptData(outbound.data(),int(outbound.size()));assert(outbound==greeting);
  }
  auto pacingProfile=[&](const char* value){std::ofstream(root/"pace.ini")<<"[Client]\nConnectHost=127.0.0.1\nAssetDirectory=/tmp\nClientSerial=1234567890123456\nClientVersion=1.02.00\nMovementRate="<<value<<"\n";return (root/"pace.ini").string();};
  assert(std::abs(load_profile(pacingProfile("0.90")).movementRate-.9)<1e-9);
  rejects([&]{load_profile(pacingProfile("0"));});rejects([&]{load_profile(pacingProfile("nan"));});rejects([&]{load_profile(pacingProfile("1.0junk"));});
 }catch(...){std::filesystem::remove_all(root);throw;}
 std::filesystem::remove_all(root);
}
