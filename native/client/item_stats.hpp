#pragma once
#include "item_catalog.hpp"
#include "inventory.hpp"
namespace mu::ui {
// Equipment groups 0..11. Pets/wings/event items have separate source rules.
struct ItemStats {unsigned damageMin=0,damageMax=0,defense=0,blocking=0,magicDefense=0,magicPower=0;std::array<unsigned,6> requirements{};bool supported=false;};
unsigned maximum_durability(const mu::client::Item&,const ItemAttribute&);
std::array<double,4> item_background(const mu::client::Item&,const ItemAttribute&);
ItemStats equipment_stats(const mu::client::Item&,const ItemAttribute&);
}
