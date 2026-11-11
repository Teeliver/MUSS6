#include "item_placement.hpp"
namespace mu::ui {
EquipmentRect equipment_rect(unsigned slot){switch(slot){
case 0:return {15,87,46,66};case 1:return {135,87,46,66};case 2:return {75,44,46,46};
case 3:return {75,87,46,66};case 4:return {75,150,46,46};case 5:return {15,150,46,46};
case 6:return {135,150,46,46};case 7:return {120,44,61,46};case 8:return {15,44,46,46};
case 9:return {54,87,28,28};case 10:return {114,150,28,28};case 11:return {54,150,28,28};default:return {};}}
int equipment_cell(double x,double y){int result=-1;for(unsigned slot:{8u,2u,7u,1u,3u,0u,5u,4u,6u,10u,9u,11u}){auto r=equipment_rect(slot);if(x>=r.x&&x<r.x+r.width&&y>=r.y&&y<r.y+r.height)result=slot;}return result;}
namespace {
bool layout(unsigned container,unsigned& base,unsigned& rows){if(container!=0&&container!=2)return false;base=container==0?12:0;rows=container==0?8:15;return true;}
}
int item_at_cell(const mu::client::ItemSlots& items,const ItemCatalog& catalog,unsigned container,unsigned cell){if(container==0&&cell<12)return items.count(cell)?int(cell):-1;unsigned base,rows;if(!layout(container,base,rows)||cell<base||cell>=base+rows*8)return -1;unsigned x=(cell-base)%8,y=(cell-base)/8;for(auto& pair:items){if(pair.first<base||pair.first>=base+rows*8)continue;auto& a=catalog.at(pair.second.index);unsigned ix=(pair.first-base)%8,iy=(pair.first-base)/8;if(x>=ix&&y>=iy&&x<ix+a.width&&y<iy+a.height)return pair.first;}return -1;}
bool can_place_item(const mu::client::ItemSlots& items,const ItemCatalog& catalog,unsigned container,unsigned cell,const mu::client::Item& item,int ignoredSlot,unsigned baseClass){if(container==0&&cell<12){auto& a=catalog.at(item.index);if(!a.width||!a.height||a.wearSlot>=12)return false;if(items.count(cell)&&int(cell)!=ignoredSlot)return false;
 bool slotAllowed=a.wearSlot==cell||(cell>=10&&a.wearSlot>=10&&a.wearSlot<12);if(baseClass>6)slotAllowed=slotAllowed||(cell<2&&a.wearSlot<2);else if(a.wearSlot==0&&cell==1)slotAllowed=((baseClass==1||baseClass==3||baseClass==6)&&!a.twoHand)||(baseClass==5&&item.index/512!=5);if(!slotAllowed)return false;if(baseClass==6&&cell==5)return false;
 if(baseClass==2&&cell==0){auto left=items.find(1);if(left!=items.end()&&ignoredSlot!=1&&left->second.index/512==4&&left->second.index!=4*512+7&&item.index!=4*512+15)return false;}
 if(cell<2){unsigned other=cell^1;auto found=items.find(other);if(found!=items.end()&&int(other)!=ignoredSlot){auto& b=catalog.at(found->second.index);unsigned pairedAmmo=4*512+(cell==0?7:15),ownAmmo=4*512+(cell==0?15:7);if((a.twoHand&&found->second.index!=pairedAmmo)||(b.twoHand&&item.index!=ownAmmo))return false;}}
 return true;}unsigned base,rows;if(!layout(container,base,rows)||cell<base||cell>=base+rows*8)return false;auto& a=catalog.at(item.index);unsigned x=(cell-base)%8,y=(cell-base)/8;if(!a.width||!a.height||x+a.width>8||y+a.height>rows)return false;for(unsigned dy=0;dy<a.height;dy++)for(unsigned dx=0;dx<a.width;dx++){int occupied=item_at_cell(items,catalog,container,base+(y+dy)*8+x+dx);if(occupied>=0&&occupied!=ignoredSlot)return false;}return true;}
}
namespace mu::ui {
int picked_item_cell(double x,double y,unsigned container,const ItemAttribute& item){if(container!=0&&container!=2)return -1;if(container==0){int slot=equipment_cell(x,y);if(slot>=0)return slot;}x-=(item.width-1)*10;y-=(item.height-1)*10;unsigned base=container==0?12:0,rows=container==0?8:15;double top=container==0?200:36;if(x<15||x>=175||y<top||y>=top+rows*20)return -1;return base+int((y-top)/20)*8+int((x-15)/20);}
}
