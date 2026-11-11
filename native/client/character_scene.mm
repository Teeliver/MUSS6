#include "character_scene.hpp"
#include "asset_images.hpp"
#include "character_resources.hpp"
#include <cstdio>
#include <stdexcept>
namespace mu::lobby {
std::vector<CharacterVisual> load_characters(const std::string& root,const mu::client::CharacterList& list){
 std::vector<CharacterVisual> result;
 for(const auto& character:list.characters){if(character.slot>=5)throw std::runtime_error("Invalid character slot");auto resources=mu::viewer::character_resources(character);auto body=mu::viewer::load_character_body(root,resources);CharacterVisual entry;entry.character=character;entry.unsupported=resources.warnings;auto visual=std::make_shared<mu::viewer::ActorVisual>();visual->effectDirectory=std::filesystem::path(root)/"Data";visual->bones=body->bones;visual->idle=body->combat[0];visual->idlePeriod=body->combatPeriods[0];visual->walk=body->combat[1];visual->walkPeriod=body->combatPeriods[1];visual->scale=.9;visual->name=character.name;for(const auto& part:body->meshes)visual->parts.push_back({part.mesh,part.image,false,part.equipment});for(const auto& attachment:body->attachments)visual->links.emplace_back(attachment.slot,attachment.visual);entry.visual=visual;result.push_back(std::move(entry));}
 return result;
}
}
