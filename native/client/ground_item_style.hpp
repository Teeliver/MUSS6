#pragma once
#include "inventory.hpp"
namespace mu::ui {
// Reference: MuMain ZzzInventory.cpp BuildGroundItemLabelDescriptor.
// This is a reference client, not proof of byte-for-byte EX603 Main parity.
struct GroundItemStyle {std::array<unsigned,3> text{255,255,255},background{0,0,0};bool bold=false;};
inline GroundItemStyle ground_item_style(const mu::client::Item& item,bool zen=false){
 GroundItemStyle s;unsigned group=item.index/512,type=item.index%512;
 bool jewel=(group==12&&type==15)||(group==14&&(type==13||type==14||type==16||type==22||type==31||type==41||(type>=42&&type<=44)))||(group==13&&type==14);
 if(zen||jewel||(group==14&&type>=17&&type<=19)){s.text={255,204,25};s.bold=!zen;return s;}
 bool wing=(group==12&&((type>=3&&type<=6)||(type>=36&&type<=43)||(type>=49&&type<=50)))||(group==13&&type==30);
 bool anySocket=false,allZero=true;for(unsigned i=7;i<12;i++){allZero&=item.wire[i]==0;anySocket|=item.wire[i]!=255;}bool socket=anySocket&&!allZero;
 if(socket)s.text={178,102,255};
 else if(item.excellent&&!wing)s.text={25,255,127};
 else if(item.level>=7)s.text={255,204,25};
 else if(item.skill||item.luck||item.option)s.text={102,178,255};
 else if(item.level==0)s.text={178,178,178};
 else if(item.level<3)s.text={229,229,229};
 else if(item.level<5)s.text={255,127,51};
 else s.text={102,178,255};
 if((item.setOption&3)==1||(item.setOption&3)==2){s.text={0,255,0};s.background={60,60,200};s.bold=true;}
 return s;
}
}
