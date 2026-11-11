#pragma once
#include <string>
#include <stdexcept>
#include <cstdio>
namespace mu::core {
// Port of native/client/character_resources.cpp; equipment render paths, not inventory icons.
inline std::string armor_filename(const std::string& family,unsigned number){char n[16];std::snprintf(n,sizeof(n),"%02u.bmd",number);return family+n;}
inline std::string armor_model(unsigned part,unsigned type,unsigned cls,unsigned tier){
 if(part>=5||cls>=7||tier>2)throw std::runtime_error("Invalid character resource identity");
 const char* parts[]={"Helm","Armor","Pant","Glove","Boot"};std::string p=parts[part];
 // ZzzCharacter::GetSkinModelIndex: MG/DL/RF have no second-class body.
 if(type==511){if((cls==3||cls==4||cls==6)&&tier==1)tier=0;return armor_filename(p+(tier==2?"Class3":tier==1?"Class2":"Class"),cls+1);}
 // RenderPartObject class-specific common armor substitutions.
 if(cls==5&&(type==10||type==11))return armor_filename(p+"ElfC",type-9);
 if(cls==6&&part!=3){unsigned common[]={5,6,8,9};for(unsigned i=0;i<4;i++)if(type==common[i])return armor_filename(p+"Monk",i+1);}
 if(part==0&&cls==4&&(type==0||type==5||type==6||type==8||type==9))return armor_filename("MaskHelmMale",type+1);
 if(part==0&&(type==15||type==20||type==23||type==32||type==37||type==47||type==48))return armor_model(part,511,cls,tier);
 if(type<=9)return armor_filename(p+"Male",type+1);
 if(type<=14)return armor_filename(p+"Elf",type-9);
 if(type>=15&&type<=28){if(type==19)return armor_filename(p+"MaleTest",20);if(type==18&&part==2)return armor_filename("t_PantMale",19);return armor_filename(p+"Male",type+1);}
 if(type>=29&&type<=33)return armor_filename("HDK_"+p+"Male",type-28);
 if(type>=34&&type<=38)return armor_filename("CW_"+p+"Male",type-33);
 if(type>=39&&type<=53)return armor_filename(p+"Male",type+1);
 if(type>=59&&type<=61&&part!=3)return armor_filename(p+"Male",type+1);
 throw std::runtime_error("Unmapped original armor model: part="+std::to_string(part)+" type="+std::to_string(type));
}
}
