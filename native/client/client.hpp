#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <optional>
#include "viewport.hpp"
#include "inventory.hpp"
namespace mu::client {
using Bytes=std::vector<std::uint8_t>;
struct Profile {
 std::string connectHost,assetDirectory,serial,version,language="Eng";
 int connectPort=44405;
 double movementRate=1.; // Explicit native-client pacing calibration; 1 = source reference.
 std::uint8_t key1=0,key2=0,mhp1=0,mhp2=0;
 bool mhp=false;
};
struct Server { unsigned code,load,type; };
struct Greeting { unsigned index,serverCode; std::string version; };
Profile load_profile(const std::string& path="");
std::vector<Server> parse_list(const Bytes&);
Greeting parse_greeting(const Bytes&);
std::vector<Server> discover(const Profile&);
std::string connect_game(const Profile&,unsigned code);
void decode_transport(Bytes&,const Profile&);
void encode_transport(Bytes&,const Profile&);
struct Character { unsigned slot,level,control; std::string name; Bytes appearance;unsigned guildStatus=255; };
struct CharacterList { unsigned classCode,warehouse; std::vector<Character> characters;unsigned deleteMaxLevel=0;bool deletePolicyReady=false; };
struct NpcState { bool replied=false,itemsReady=false,moneyReady=false;std::uint32_t warehouseMoney=0,inventoryMoney=0;unsigned lockState=0;unsigned result=0,npcClass=0,contribution=0;unsigned itemCount=0;std::shared_ptr<const ItemSlots> items; };
struct ServerNotice {unsigned type=0,count=0,opacity=0,delay=0,color=0,speed=0;std::string message;};
std::optional<ServerNotice> parse_notice(const Bytes&);
struct VitalState {std::uint32_t hp=0,maxHp=0,mp=0,maxMp=0,ag=0,maxAg=0,sd=0,maxSd=0,experience=0,nextExperience=0;unsigned level=0;bool ready=false;};
bool apply_character_attributes(EquipmentAttributes&,const Bytes&);
struct DamageEvent {std::uint64_t revision=0;unsigned target=0,damage=0,type=0;};
struct CombatState {std::vector<DamageEvent> hits;std::uint64_t castRevision=0,damageRevision=0,levelRevision=0,kills=0;unsigned skill=0,castTarget=0,target=0,damage=0,damageType=0;};
bool apply_combat_packet(CombatState&,const Bytes&,unsigned selfIndex);
Bytes normal_attack_packet(unsigned target,unsigned direction);
Bytes skill_attack_packet(unsigned skill,unsigned target);
struct Receipt {std::uint64_t revision=0;unsigned kind=0,amount=0;Item item;};
struct WorldState {std::vector<Receipt> receipts;std::uint64_t receiptRevision=0;CombatState combat; std::map<unsigned,unsigned> skills;bool skillsReady=false;EquipmentAttributes attributes;VitalState vitals;std::uint32_t money=0;bool moneyReady=false; std::shared_ptr<const InventorySnapshot> inventory;NpcState npc; std::shared_ptr<const ViewportSnapshot> viewport; unsigned playerIndex=0,map=0,x=0,y=0,direction=0,physicalSpeed=0,magicSpeed=0; bool statsReady=false,inventoryReady=false;int bootsLevel=-1; std::uint64_t packets=0; };
int parse_boots_level(const Bytes&);
Bytes path_packet(unsigned x,unsigned y,const std::vector<unsigned>& directions);
Bytes npc_talk_packet(unsigned index);
Bytes move_packet(unsigned x,unsigned y,unsigned direction);
struct WorldEntry { unsigned map,x,y,direction; Bytes packet;unsigned baseClass=0;Character character; };
CharacterList parse_characters(const Bytes&);
Bytes delete_character_packet(const std::string& name,const std::string& personalCode);
Bytes login_packet(const Profile&,const std::string&,const std::string&,std::uint32_t tick);
std::string login_result(unsigned);
Bytes logout_packet(unsigned type);
class Session {
 struct Impl; std::unique_ptr<Impl> impl;
public:
 Session(const Profile&,unsigned code);
 ~Session();
 unsigned login(const std::string& account,const std::string& password);
 CharacterList characters();
 unsigned create(const std::string& name,unsigned classCode);
 unsigned erase(const std::string& name,const std::string& personalCode);
 WorldEntry select(const std::string& name);
 void keepalive();
 void talk(unsigned index);
 void closeTalk();
 bool moveItem(const ItemMove&);
 bool addStat(unsigned type);
 unsigned buyItem(unsigned shopSlot);
 bool sellItem(unsigned inventorySlot);
 bool repairItem(unsigned inventorySlot=255);
 bool useBook(unsigned inventorySlot);
 void attack(unsigned target,unsigned direction);
 void castSkill(unsigned skill,unsigned target);
 void castAreaSkill(unsigned skill,unsigned direction);
 bool dropItem(unsigned slot);
 bool pickItem(unsigned index);
 WorldState pump();
 WorldState move(unsigned direction);
 WorldState movePath(unsigned x,unsigned y,const std::vector<unsigned>& directions);
 WorldState state() const;
 void disconnect();
 // Original F1:02 targets: 0 exit, 1 character selection, 2 server selection.
 void requestLogout(unsigned type);
 void setNoticeHandler(std::function<void(const ServerNotice&)>);
 Session(const Session&)=delete;
 Session& operator=(const Session&)=delete;
};
}
