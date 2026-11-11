#pragma once
#include "client.hpp"
#include "assets.hpp"
#include "entity_assets.hpp"
#include "item_catalog.hpp"
#include <memory>
namespace mu::viewer {
// Shared by character selection and world entry. Armor order is server slots 2..6.
struct CharacterResources {
 unsigned baseClass=0,tier=0,idle=1,walk=15,run=25;
 std::array<unsigned,5> armor{{511,511,511,511,511}};
 std::array<std::optional<mu::client::Item>,3> attachments;
 std::array<std::string,5> models;std::array<std::optional<mu::client::Item>,5> armorItems;
 std::vector<std::string> warnings;
 std::string key()const;
};
CharacterResources base_character_resources(unsigned baseClass);
CharacterResources character_resources(const mu::client::Character&,const mu::client::InventorySnapshot* inventory=nullptr);
std::string armor_model(unsigned part,unsigned type,unsigned baseClass,unsigned tier);
struct CharacterMesh {mu::assets::Mesh mesh;std::shared_ptr<const mu::assets::Image> image;unsigned part;std::optional<mu::client::Item> equipment;};
struct CharacterAttachment {unsigned slot;mu::client::Item item;std::shared_ptr<const ActorVisual> visual;};
std::array<unsigned,3> equipped_actions(const CharacterResources&,const mu::ui::ItemCatalog&);
struct CharacterBody {
 std::array<unsigned,6> combatActions{},combatPeriods{};
 std::array<std::vector<std::vector<mu::assets::Bone>>,6> combat;
 std::vector<CharacterAttachment> attachments;
 std::vector<mu::assets::Bone> bones;
 std::vector<std::vector<mu::assets::Bone>> idle,walk,run;
 std::array<unsigned,3> periods{};
 std::vector<CharacterMesh> meshes;
};
std::shared_ptr<const CharacterBody> load_character_body(const std::string& root,const CharacterResources&);
}
