#pragma once
#include "assets.hpp"
#include "inventory.hpp"
#include <memory>
#include <optional>
#include <map>
#include <vector>
namespace mu::viewer {
constexpr unsigned dwPlayerVisualKey=0x10000; // Reserved outside the 16-bit monster/skin wire domain.
struct ActorPart { mu::assets::Mesh mesh;std::shared_ptr<const mu::assets::Image> image;bool additive=false;std::optional<mu::client::Item> equipment; };
struct ActorVisual {
 std::filesystem::path effectDirectory;unsigned type=0;bool npc=false;float scale=1,opacity=1,altitude=0;double moveSpeed=2.5,walkKeySpeed=.34,idleKeySpeed=.25;
 std::vector<std::pair<unsigned,std::shared_ptr<const ActorVisual>>> links;
 std::string name;std::vector<ActorPart> parts;std::vector<mu::assets::Bone> bones;
 std::array<std::vector<std::vector<mu::assets::Bone>>,2> attack;std::array<unsigned,2> attackPeriod{};
 std::vector<std::vector<mu::assets::Bone>> idle,walk,death;unsigned idlePeriod=0,walkPeriod=0,deathPeriod=0;
};
using ActorCatalog=std::map<unsigned,std::shared_ptr<const ActorVisual>>;
// Model names are an explicit CreateMonster/OpenNpc mapping, never type+1.
ActorCatalog load_actor_catalog(const std::filesystem::path& data);
}
