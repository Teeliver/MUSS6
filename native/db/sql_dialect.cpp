#include "sql_dialect.hpp"
#include "known_queries.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <vector>

namespace mu::db {
namespace {
enum class Kind { Word, Number, String, Identifier, Symbol, Parameter };
struct Token { Kind kind; std::string text; };
std::string Upper(std::string_view s) {
  std::string out(s);
  for (char& c : out) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return out;
}
std::vector<Token> Lex(std::string_view sql) {
  std::vector<Token> tokens;
  for (std::size_t i = 0; i < sql.size();) {
    const unsigned char c = sql[i];
    if (std::isspace(c)) { ++i; continue; }
    const auto start = i;
    if (c == '\'') {
      ++i;
      bool ended = false;
      while (i < sql.size()) {
        if (sql[i++] == '\'') {
          if (i < sql.size() && sql[i] == '\'') { ++i; continue; }
          ended = true; break;
        }
      }
      if (!ended) throw std::invalid_argument("unterminated SQL string");
      tokens.push_back({Kind::String, std::string(sql.substr(start, i - start))});
    } else if (c == '[') {
      ++i;
      std::string identifier;
      bool ended = false;
      while (i < sql.size()) {
        if (sql[i] == ']') { ++i; ended = true; break; }
        if (sql[i] == '`') throw std::invalid_argument("invalid bracketed identifier");
        identifier += sql[i++];
      }
      if (!ended || identifier.empty()) throw std::invalid_argument("invalid bracketed identifier");
      tokens.push_back({Kind::Identifier, "`" + identifier + "`"});
    } else if (std::isalpha(c) || c == '_') {
      while (i < sql.size() && (std::isalnum(static_cast<unsigned char>(sql[i])) || sql[i] == '_')) ++i;
      tokens.push_back({Kind::Word, std::string(sql.substr(start, i - start))});
    } else if (std::isdigit(c)) {
      while (i < sql.size() && (std::isalnum(static_cast<unsigned char>(sql[i])) || sql[i] == '.')) ++i;
      tokens.push_back({Kind::Number, std::string(sql.substr(start, i - start))});
    } else {
      if (c == ';' || c == '`' || c == '@' || c == '"' ||
          (i + 1 < sql.size() && ((c == '-' && sql[i + 1] == '-') || (c == '/' && sql[i + 1] == '*')))) {
        throw std::invalid_argument("unsupported SQL statement boundary or token");
      }
      tokens.push_back({c == '?' ? Kind::Parameter : Kind::Symbol, std::string(1, sql[i++])});
    }
  }
  return tokens;
}
bool At(const std::vector<Token>& t, std::size_t at, std::string_view value) {
  return at < t.size() && Upper(t[at].text) == Upper(value);
}
std::string Join(const std::vector<Token>& t) {
  std::string result;
  for (const auto& token : t) {
    if (!result.empty()) result += ' ';
    result += token.text;
  }
  return result;
}
bool Procedure(std::string_view name, std::size_t arity) {
  return std::any_of(std::begin(kKnownProcedures), std::end(kKnownProcedures), [&](const KnownProcedure& p) { return Upper(p.name) == Upper(name) && p.arity == arity; });
}
const std::vector<std::string> keywords = {
  "SELECT","FROM","WHERE","UPDATE","INSERT","INTO","VALUES","SET","DELETE","AND","OR","NOT","NULL","IS","AS","ORDER","BY","DESC","ASC","GROUP","HAVING","IN","LIKE","DISTINCT","UNION","ALL","LIMIT",
  "COUNT","ROW_NUMBER","OVER","CURRENT_TIMESTAMP","UNHEX","REPEAT",
};
} // namespace

bool IsKnownQueryFormat(std::string_view format) {
  return std::any_of(std::begin(kKnownFormats), std::end(kKnownFormats), [&](const char* known) { return format == known; });
}

std::string PortablePrintfFormat(std::string_view source) {
  std::string out;
  for (std::size_t i = 0; i < source.size(); ++i) {
    if (source[i] == '%' && i + 1 < source.size()) {
      out += source[i++];
      if (source[i] == '%') { out += source[i]; continue; }
      while (i < source.size() && std::string_view("-+ #0.123456789*").find(source[i]) != std::string_view::npos) out += source[i++];
      if (source.substr(i, 3) == "I64") { out += "ll"; i += 3; }
      if (i == source.size() || source[i] == 'n') throw std::invalid_argument("invalid printf SQL conversion");
    }
    if (i < source.size()) out += source[i];
  }
  return out;
}

TranslatedQuery TranslateQuery(std::string_view sourceFormat, std::string_view sql) {
  if (!IsKnownQueryFormat(sourceFormat)) throw std::invalid_argument("SQL format is outside the original Join/Data query catalog");
  auto t = Lex(sql);
  if (t.empty()) throw std::invalid_argument("empty SQL query");
  const auto placeholders = std::count_if(t.begin(), t.end(), [](const Token& token) { return token.kind == Kind::Parameter; });
  std::size_t first = At(t, 0, "EXEC") ? 1 : 0;
  if (first < t.size() && t[first].kind == Kind::Word && Upper(t[first].text).rfind("WZ_", 0) == 0) {
    const std::string name = t[first].text;
    std::size_t arity = first + 1 < t.size() ? 1 : 0;
    int depth = 0;
    for (std::size_t i = first + 1; i < t.size(); ++i) {
      if (At(t, i, "(")) ++depth;
      if (At(t, i, ")") && --depth < 0) throw std::invalid_argument("unbalanced procedure argument");
      if (depth == 0 && At(t, i, ",")) ++arity;
    }
    if (depth != 0 || !Procedure(name, arity)) throw std::invalid_argument("unknown procedure signature in source query");
    std::vector<Token> arguments(t.begin() + first + 1, t.end());
    return {"CALL `" + name + "`(" + Join(arguments) + ")", static_cast<std::size_t>(placeholders)};
  }
  if (first != 0 || (!At(t, 0, "SELECT") && !At(t, 0, "INSERT") && !At(t, 0, "UPDATE") && !At(t, 0, "DELETE"))) throw std::invalid_argument("unsupported source SQL operation");

  std::string limit;
  if (At(t, 0, "SELECT") && At(t, 1, "TOP")) {
    if (!At(t, 2, "6") && !At(t, 2, "50")) throw std::invalid_argument("unsupported TOP query");
    limit = t[2].text;
    t.erase(t.begin() + 1, t.begin() + 3);
  }
  for (std::size_t i = 0; i < t.size(); ++i) {
    if (At(t, i, "GETDATE") && At(t, i + 1, "(") && At(t, i + 2, ")")) {
      t[i] = {Kind::Word, "CURRENT_TIMESTAMP"};
      t.erase(t.begin() + i + 1, t.begin() + i + 3);
    } else if (At(t, i, "CONVERT")) {
      // Only Warehouse.cpp's exact empty-item byte pattern is supported.
      if (!(At(t,i+1,"(") && At(t,i+2,"VARBINARY") && At(t,i+3,"(") &&
            i+4<t.size() && t[i+4].kind==Kind::Number && At(t,i+5,")") && At(t,i+6,",") &&
            At(t,i+7,"REPLICATE") && At(t,i+8,"(") && At(t,i+9,"CHAR") && At(t,i+10,"(") &&
            At(t,i+11,"0xFF") && At(t,i+12,")") && At(t,i+13,",") && i+14<t.size() &&
            t[i+14].text==t[i+4].text && At(t,i+15,")") && At(t,i+16,")"))) throw std::invalid_argument("unsupported CONVERT expression");
      const auto size = std::stoul(t[i+4].text);
      if (size == 0 || size > 524288) throw std::invalid_argument("invalid warehouse binary length");
      t[i] = {Kind::Identifier, "UNHEX(REPEAT('FF'," + std::to_string(size) + "))"};
      t.erase(t.begin() + i + 1, t.begin() + i + 17);
    }
  }
  // SQL Server accepts Ranking,*; MySQL requires a qualified wildcard in this position.
  for (std::size_t i = 1; i + 2 < t.size(); ++i) {
    if (At(t, i, "*") && At(t, i - 1, ",") && At(t, i + 1, "FROM") && t[i + 2].kind == Kind::Word) {
      t[i] = {Kind::Identifier, "`" + t[i + 2].text + "`.*"};
    }
  }
  for (auto& token : t) {
    if (token.kind != Kind::Word) continue;
    const auto word = Upper(token.text);
    if (word == "EXEC" || word == "TOP" || word == "CONVERT" || word == "GETDATE" || word == "REPLICATE" || word == "VARBINARY") throw std::invalid_argument("unsupported SQL Server syntax remains");
    if (std::find(keywords.begin(), keywords.end(), word) == keywords.end()) token.text = "`" + token.text + "`";
  }
  auto translated = Join(t);
  if (!limit.empty()) translated += " LIMIT " + limit;
  return {translated, static_cast<std::size_t>(placeholders)};
}
} // namespace mu::db
