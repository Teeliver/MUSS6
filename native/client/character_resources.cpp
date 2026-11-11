#include "character_resources.hpp"
#include <sstream>
#include <stdexcept>
#include <cstdio>
namespace mu::viewer {namespace {
std::string filename(const std::string& family,unsigned number){char n[16];std::snprintf(n,sizeof(n),"%02u.bmd",number);return family+n;}
}
std::string armor_model(unsigned part,unsigned type,unsigned cls,unsigned tier){
 if(part>=5||cls>=7||tier>2)throw std::runtime_error("Invalid character resource identity");
 const char* parts[]={"Helm","Armor","Pant","Glove","Boot"};std::string p=parts[part];
 // ZzzCharacter::GetSkinModelIndex: MG/DL/RF have no second-class body.
 if(type==511){if((cls==3||cls==4||cls==6)&&tier==1)tier=0;return filename(p+(tier==2?"Class3":tier==1?"Class2":"Class"),cls+1);}
 // RenderPartObject class-specific common armor substitutions.
 if(cls==5&&(type==10||type==11))return filename(p+"ElfC",type-9);
 if(cls==6&&part!=3){unsigned common[]={5,6,8,9};for(unsigned i=0;i<4;i++)if(type==common[i])return filename(p+"Monk",i+1);}
 if(part==0&&cls==4&&(type==0||type==5||type==6||type==8||type==9))return filename("MaskHelmMale",type+1);
 if(part==0&&(type==15||type==20||type==23||type==32||type==37||type==47||type==48))return armor_model(part,511,cls,tier);
 if(type<=9)return filename(p+"Male",type+1);
 if(type<=14)return filename(p+"Elf",type-9);
 if(type>=15&&type<=28){if(type==19)return filename(p+"MaleTest",20);if(type==18&&part==2)return filename("t_PantMale",19);return filename(p+"Male",type+1);}
 if(type>=29&&type<=33)return filename("HDK_"+p+"Male",type-28);
 if(type>=34&&type<=38)return filename("CW_"+p+"Male",type-33);
 if(type>=39&&type<=53)return filename(p+"Male",type+1);
 if(type>=59&&type<=61&&part!=3)return filename(p+"Male",type+1);
 throw std::runtime_error("Unmapped original armor model: part="+std::to_string(part)+" type="+std::to_string(type));
}
CharacterResources character_resources(const mu::client::Character& c,const mu::client::InventorySnapshot* inventory){
 if(c.appearance.size()!=18)throw std::runtime_error("Character appearance must be EX603 CharSet[18]");const auto& a=c.appearance;CharacterResources r;r.baseClass=a[0]>>5;if(r.baseClass>6)throw std::runtime_error("Invalid EX603 character class");r.tier=(a[0]&28)>=28?2:(a[0]&16)?1:0;
 r.idle=r.baseClass==2?2:r.baseClass==5?3:1;r.walk=(r.baseClass==2||r.baseClass==5)?16:15;
 if(r.baseClass==6)r.warnings.push_back("Rage Fighter idle action not verified for supplied 284-action Player.bmd; male idle retained");
 r.armor={{unsigned((a[3]>>4)|((a[9]&128)>>3)|((a[13]&15)<<5)),unsigned((a[3]&15)|((a[9]&64)>>2)|((a[14]&240)<<1)),unsigned((a[4]>>4)|((a[9]&32)>>1)|((a[14]&15)<<5)),unsigned((a[4]&15)|(a[9]&16)|((a[15]&240)<<1)),unsigned((a[5]>>4)|((a[9]&8)<<1)|((a[15]&15)<<5))}};
 // GameServer::DGCharacterListRecv packs seven 3-bit levels in CharSet[6..8].
 // LevelConvert reconstructs visual grades; the lobby does not receive exact item bytes.
 if(!inventory||!inventory->ready){unsigned packed=(unsigned(a[6])<<16)|(unsigned(a[7])<<8)|a[8];constexpr unsigned grades[]={0,3,5,7,9,11,13,15};for(unsigned i=0;i<5;i++)if(r.armor[i]!=511){mu::client::Item item;item.index=(i+7)*512+r.armor[i];item.level=grades[(packed>>((i+2)*3))&7];item.excellent=(a[10]&(128>>i))?1:0;item.setOption=(a[11]&(128>>i))?1:0;r.armorItems[i]=item;}}
 if(inventory&&inventory->ready)for(unsigned i=0;i<5;i++){auto found=inventory->items.find(i+2);r.armor[i]=511;if(found!=inventory->items.end()){if(found->second.index/512!=i+7)throw std::runtime_error("Wrong item group in equipped armor slot");r.armor[i]=found->second.index%512;r.armorItems[i]=found->second;if(found->second.excellent)r.warnings.push_back("Excellent armor-specific particles not ported");}}
 for(unsigned i=0;i<5;i++)r.models[i]=armor_model(i,r.armor[i],r.baseClass,r.tier);
 if(!inventory||!inventory->ready){unsigned packed=(unsigned(a[6])<<16)|(unsigned(a[7])<<8)|a[8];constexpr unsigned grades[]={0,3,5,7,9,11,13,15};for(unsigned hand=0;hand<2;hand++){unsigned index=unsigned(a[1+hand])|((unsigned(a[12+hand])&240)<<4);if(index==0xFFF)continue;mu::client::Item item;item.index=index;item.level=grades[(packed>>(hand*3))&7];unsigned flag=hand==0?4:2;item.excellent=(a[10]&flag)?1:0;item.setOption=(a[11]&flag)?1:0;r.attachments[hand]=item;}}
 if(inventory&&inventory->ready){for(unsigned i=0;i<3;i++){unsigned slot=i==2?7:i;auto found=inventory->items.find(slot);if(found!=inventory->items.end())r.attachments[i]=found->second;}}
 if((a[5]&12)!=0)r.warnings.push_back("CharSet-only wing preview not ported; world uses authoritative slot 7");
 return r;
}
std::array<unsigned,3> equipped_actions(const CharacterResources& r,const mu::ui::ItemCatalog& catalog){std::array<unsigned,3> out={r.idle,r.walk,r.run};auto right=r.attachments[0],left=r.attachments[1];if(right){unsigned group=right->index/512,type=right->index%512;bool two=catalog.at(right->index).twoHand;if(group<=2){if(two){if(group==0&&(type==21||type==23||type==25||type==31))out={142,143,144};else out={5,18,28};}else out={4,17,(left&&left->index/512<=2)?27u:26u};return out;}if(group==3){out={(type==1||type==2)?6u:two?7u:4u,type==1?19u:20u,29};return out;}if(group==5){if(type>=14&&type<=20)out={10,23,32};else out={two?7u:4u,two?20u:17u,two?29u:26u};return out;}}
 // GetEquipedBowType uses the occupied hand, not a guessed model family.
 if(left&&left->index/512==4&&left->index%512!=7)return {8,21,30};if(right&&right->index/512==4&&right->index%512>=8&&right->index%512!=15)return {9,22,31};return out;}
CharacterResources base_character_resources(unsigned cls){mu::client::Character c{};c.appearance.assign(18,255);c.appearance[0]=cls<<5;c.appearance[5]=0xF3;c.appearance[9]=0xF8;c.appearance[10]=c.appearance[11]=c.appearance[16]=c.appearance[17]=0;c.appearance[12]=0xF0;return character_resources(c);}
std::string CharacterResources::key()const{std::ostringstream out;out<<baseClass<<":"<<tier<<":"<<idle<<":"<<walk<<":"<<run;for(unsigned i=0;i<models.size();i++){out<<":"<<models[i];if(armorItems[i])out<<":"<<armorItems[i]->level<<":"<<armorItems[i]->excellent<<":"<<armorItems[i]->setOption;}for(unsigned i=0;i<attachments.size();i++){out<<":attachment"<<i;if(attachments[i])out<<":"<<attachments[i]->index<<":"<<attachments[i]->level<<":"<<attachments[i]->excellent<<":"<<attachments[i]->setOption;}return out.str();}
}
