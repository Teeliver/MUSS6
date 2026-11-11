#pragma once
#include "assets.hpp"
#include <map>
namespace mu::ui {
using Texts=std::map<std::uint32_t,std::string>;
Texts decode_texts(const mu::assets::Bytes& bytes);
std::filesystem::path local_data_file(const std::filesystem::path& data,const std::string& language,const std::string& stem);
Texts load_texts(const std::filesystem::path& data,const std::string& language);
std::string format_gold(std::uint32_t amount);
std::array<unsigned,3> gold_color(std::uint32_t amount);
}
