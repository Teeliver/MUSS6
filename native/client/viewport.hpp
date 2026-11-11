#pragma once
#include <array>
#include "inventory.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <vector>
namespace mu::client {
enum class EntityKind { Player,Monster,Summon,ChangedPlayer };
struct Entity {
 unsigned index=0,type=0,x=0,y=0,tx=0,ty=0,direction=0,pkLevel=0,action=0,target=0;
 bool created=false,teleport=false,dead=false;std::string name;
 std::array<std::uint8_t,18> appearance{};std::vector<std::uint8_t> effects;
 EntityKind kind=EntityKind::Monster;std::uint64_t revision=0,actionRevision=0;
};
struct GroundItem {unsigned index=0,x=0,y=0,money=0;Item item;std::uint64_t revision=0;};
struct ViewportSnapshot {std::map<unsigned,GroundItem> items; std::uint64_t revision=0;std::map<unsigned,Entity> entities,recentDead; };
bool is_unarmored_dw(const Entity&);
// Decodes only EX603 entity packets. Entire packet is validated before mutation.
// Unknown packet heads and updates for entities outside the viewport are ignored.
bool apply_viewport_packet(ViewportSnapshot&,const std::vector<std::uint8_t>&,unsigned selfIndex);
}
