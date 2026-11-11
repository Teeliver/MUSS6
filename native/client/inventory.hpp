#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <vector>
namespace mu::client {
// Wire item is ItemManager::ItemByteConvert (12 bytes), not the DB's 16 bytes.
struct EquipmentAttributes {std::array<std::uint32_t,5> base{},bonus{};unsigned level=0,baseClass=0,tier=0,points=0;std::array<std::uint32_t,8> combat{};bool ready=false,combatReady=false;};
struct Item {unsigned index=0,level=0,durability=0,option=0,excellent=0,setOption=0;bool skill=false,luck=false,extended=false;std::array<std::uint8_t,12> wire{};};
using ItemSlots=std::map<unsigned,Item>;
struct InventorySnapshot {std::uint64_t revision=0;bool ready=false;ItemSlots items;};
Item decode_item(const std::array<std::uint8_t,12>&);
ItemSlots decode_item_list(const std::vector<std::uint8_t>& packet,unsigned headerSize,unsigned slotLimit);
bool apply_inventory_packet(InventorySnapshot&,const std::vector<std::uint8_t>&);
}
namespace mu::client {
struct ItemMove {unsigned sourceContainer=0,sourceSlot=0,targetContainer=0,targetSlot=0;};
std::vector<std::uint8_t> item_move_packet(const ItemMove&,const Item&);
bool commit_item_move(InventorySnapshot&,ItemSlots&,const ItemMove&,const Item&,const std::vector<std::uint8_t>& reply);
}
