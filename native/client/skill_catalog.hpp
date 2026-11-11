#pragma once
#include "assets.hpp"
namespace mu::ui {
struct SkillInfo {std::string name;unsigned level=0,damage=0,mana=0,ag=0,range=0;};
std::vector<SkillInfo> load_skill_catalog(const std::filesystem::path& data,const std::string& language);
}
