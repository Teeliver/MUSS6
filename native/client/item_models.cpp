#include "item_models.hpp"
#include <cstdio>
namespace mu::viewer {
std::filesystem::path item_model_path(const std::filesystem::path& data,const mu::client::Item& item){unsigned group=item.index/512,type=item.index%512;const char* family=nullptr;unsigned number=type+1;auto directory=data/"Item";
 if(group==0&&type<=21)family="Sword";if(group==1&&type<9)family="Axe";if(group==2&&type<13)family="Mace";if(group==3&&type<11)family="Spear";if(group==4&&type<7)family="Bow";if(group==4&&type>=8&&type<15){family="CrossBow";number=type-7;}if(group==5&&(type<12||(type>=14&&type<=20)))family="Staff";if(group==6&&type<17)family="Shield";
 if(group==4&&type==7)return directory/"Arrows01.bmd";if(group==4&&type==15)return directory/"Arrows02.bmd";
 if(group==2&&type==13)return directory/"Saint.bmd";
 if(group==12){if(type<=6){family="Wing";number=type+1;}if(type>=36&&type<=39){family="Wing";number=type-28;}if(type==40)return directory/"DarkLordRobe02.bmd";if(type==49||type==50){family="Wing";number=type+1;}if(type==135)return directory/"Wing50.bmd";if(type>=41&&type<=43){family="Wing";number=type+1;}}
 if(group==15&&type<19){char name[32];std::snprintf(name,sizeof(name),"Book%02u.bmd",type+1);return directory/name;}
 if(group==14&&type==15)return directory/"Gold01.bmd";
 if(group==14&&(type==13||type==14||type==16||type==22)){unsigned n=type==13?1:type==14?2:type==16?3:22;char name[32];std::snprintf(name,sizeof(name),"Jewel%02u.bmd",n);return directory/name;}
 if(group==12&&type==15)return directory/"Jewel15.bmd";
 if(group==14&&type==8)return directory/"Antidote01.bmd";
 if(group==14&&type==9)return directory/"Beer01.bmd";
 if(group==14&&type==10)return directory/"Scroll01.bmd";
 if(group==14&&type>=17&&type<=19){char name[32];std::snprintf(name,sizeof(name),"Devil%02u.bmd",type-17);return directory/name;}
 if(group==13&&type>=16&&type<=18){char name[48];std::snprintf(name,sizeof(name),"EventBloodCastle%02u.bmd",type-16);return directory/name;}
 if(group==13&&type==29)return directory/"EventChaosCastle.bmd";
 if(group==13&&type==49)return directory/"scrollpaper.bmd";
 if(group==13&&type==50)return directory/"oath.bmd";
 if(group==13&&type==30)return directory/"DarkLordRobe.bmd";
 if(group>=7&&group<=11&&type<=16&&!(group==7&&type==15)){const char* male[]={"HelmMale","ArmorMale","PantMale","GloveMale","BootMale"},*elf[]={"HelmElf","ArmorElf","PantElf","GloveElf","BootElf"};family=type>=10&&type<15?elf[group-7]:male[group-7];number=type>=10&&type<15?type-9:type+1;directory=data/"Player";}
 if(group==13&&type==20){if(item.level==0)return directory/"MagicRing00.bmd";if(item.level<=3)return directory/"RingOfLordEvent00.bmd";return {};}
 if(group==13&&type>=8&&type<=10){family="Ring";number=type==9?2:1;}if(group==13&&(type==12||type==13)){family="Necklace";number=type-11;}if(group==14&&type<7)family="Potion";
 if(group==0){const char* special[]={"HDK_Sword","HDK_Sword2","CW_Sword","CW_Sword2","Sword_27","Sword_28","Sword_29"};if(type>=22&&type<=28)return directory/(std::string(special[type-22])+".bmd");if(type==31)return directory/"Sword32.bmd";}
 if(group==4){if(type==16)return directory/"CrossBow17.bmd";if(type==17)return directory/"Bow18.bmd";if(type==18)return directory/"Bow19.bmd";if(type==19)return directory/"CrossBow20.bmd";if(type==20)return directory/"Bow20.bmd";if(type==21)return directory/"HDK_Bow.bmd";if(type==22)return directory/"CW_Bow.bmd";if(type==23)return directory/"Bow_24.bmd";}
 if(group==5){if(type==12)return directory/"HDK_Staff.bmd";if(type==13)return directory/"CW_Staff.bmd";if(type>=30&&type<=32)return directory/("Staff_"+std::to_string(type+1)+".bmd");}
 if(group==6&&type>=17&&type<=20)return directory/("Shield_"+std::to_string(type+1)+".bmd");
 if(!family)return {};char name[96];std::snprintf(name,sizeof(name),"%s%02u.bmd",family,number);return directory/name;
}
}
