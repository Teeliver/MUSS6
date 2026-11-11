#pragma once
#include <array>
#include <cstdint>
namespace mu {
// Server-computed item requirements: STR, DEX, VIT, ENE, LEAD, level.
inline bool meets_book_requirements(const std::array<std::int64_t,6>& actual,const std::array<std::int64_t,6>& required){for(unsigned i=0;i<6;i++)if(actual[i]<required[i])return false;return true;}
}
