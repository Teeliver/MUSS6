#pragma once
#include "assets.hpp"
#include <memory>
namespace mu::ui {
struct ItemAttribute {std::string name;unsigned width=0,height=0,wearSlot=0,baseDurability=0,magicDurability=0;std::array<unsigned,7> classes{}; bool twoHand=false; unsigned baseLevel=0,damageMin=0,damageMax=0,blocking=0,defense=0,magicDefense=0,weaponSpeed=0,walkSpeed=0,magicPower=0; std::array<unsigned,6> requirements{};};
using ItemCatalog=std::vector<ItemAttribute>;
ItemCatalog decode_item_catalog(const mu::assets::Bytes&);
std::shared_ptr<const ItemCatalog> load_item_catalog(const std::filesystem::path& data,const std::string& language);
}
