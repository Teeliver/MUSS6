#pragma once
#include "client.hpp"
#include "entity_assets.hpp"
namespace mu::lobby {
struct CharacterVisual {mu::client::Character character;std::shared_ptr<const mu::viewer::ActorVisual> visual;std::vector<std::string> unsupported;};
std::vector<CharacterVisual> load_characters(const std::string& root,const mu::client::CharacterList& list);
}
