#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace mu::db {
struct SqlColumn { std::string name; bool binary = false; };
using SqlCell = std::optional<std::vector<unsigned char>>;
struct SqlResult {
  std::vector<SqlColumn> columns;
  std::vector<std::vector<SqlCell>> rows;
  std::size_t next = 0;
  const std::vector<SqlCell>* current = nullptr;
  void Clear() { columns.clear(); rows.clear(); next = 0; current = nullptr; }
  bool Fetch() { current = next < rows.size() ? &rows[next++] : nullptr; return current != nullptr; }
  int Find(const char* name) const {
    if (!name) return -1;
    auto equal = [](std::string a, std::string b) {
      auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
      std::transform(a.begin(), a.end(), a.begin(), lower);
      std::transform(b.begin(), b.end(), b.begin(), lower);
      return a == b;
    };
    for (std::size_t i = 0; i < columns.size(); ++i) if (equal(columns[i].name, name)) return static_cast<int>(i);
    return -1;
  }
  const SqlCell* Cell(int index) const {
    return current && index >= 0 && static_cast<std::size_t>(index) < current->size() ? &(*current)[index] : nullptr;
  }
  std::string Text(int index) const {
    const auto* cell = Cell(index);
    return cell && cell->has_value() ? std::string(cell->value().begin(), cell->value().end()) : std::string();
  }
  std::int64_t Integer64(int index) const { return index < 0 ? -1 : std::strtoll(Text(index).c_str(), nullptr, 10); }
  int Integer(int index) const { return static_cast<std::int32_t>(static_cast<std::uint32_t>(Integer64(index))); }
  float Float(int index) const { return index < 0 ? -1.0f : std::strtof(Text(index).c_str(), nullptr); }
  void String(int index, char* buffer, int size) const {
    if (!buffer || size <= 0) return;
    std::memset(buffer, 0, static_cast<std::size_t>(size));
    const auto value = Text(index);
    std::memcpy(buffer, value.data(), std::min(value.size(), static_cast<std::size_t>(size - 1)));
  }
  void Binary(int index, unsigned char* buffer, int size) const {
    if (!buffer || size <= 0) return;
    std::memset(buffer, 0, static_cast<std::size_t>(size));
    const auto* cell = Cell(index);
    if (cell && cell->has_value()) std::memcpy(buffer, cell->value().data(), std::min(cell->value().size(), static_cast<std::size_t>(size)));
  }
};
} // namespace mu::db
