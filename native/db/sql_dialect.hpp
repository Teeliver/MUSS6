#pragma once
#include <cstddef>
#include <string>
#include <string_view>

namespace mu::db {
struct TranslatedQuery { std::string sql; std::size_t placeholders = 0; };
bool IsKnownQueryFormat(std::string_view format);
std::string PortablePrintfFormat(std::string_view format);
TranslatedQuery TranslateQuery(std::string_view sourceFormat, std::string_view formattedQuery);
} // namespace mu::db
