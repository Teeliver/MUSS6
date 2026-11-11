#pragma once
#include "item_catalog.hpp"
#include "inventory.hpp"
namespace mu::ui {
struct EquipmentRect {double x=0,y=0,width=0,height=0;};
EquipmentRect equipment_rect(unsigned slot);
int equipment_cell(double x,double y);
int item_at_cell(const mu::client::ItemSlots&,const ItemCatalog&,unsigned container,unsigned cell);
bool can_place_item(const mu::client::ItemSlots&,const ItemCatalog&,unsigned container,unsigned cell,const mu::client::Item&,int ignoredSlot=-1,unsigned baseClass=~0u);
}
namespace mu::ui {int picked_item_cell(double x,double y,unsigned container,const ItemAttribute&);}
