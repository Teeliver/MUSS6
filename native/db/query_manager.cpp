#include "query_manager.hpp"
#include "sql_dialect.hpp"
#include "sql_result.hpp"
#include "tls_error.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <string_view>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>
#include <stdexcept>
#if defined(MU_HAVE_MYSQL) && MU_HAVE_MYSQL
#include <mysql.h>
#include <errmsg.h>
static_assert(CR_SSL_CONNECTION_ERROR == mu::db::kMySqlTlsClientError, "MySQL TLS client error code changed");
#endif

namespace {
constexpr SQLRETURN kSuccess = 0, kNoData = 100;
struct Parameter { std::vector<unsigned char> bytes; bool binary = false; };
int Hex(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}
std::string Env(const char* name) { const auto* value = std::getenv(name); return value ? value : ""; }
#if defined(MU_HAVE_MYSQL) && MU_HAVE_MYSQL
void EnsureMySqlThread() {
  struct State { State() { mysql_thread_init(); } ~State() { mysql_thread_end(); } };
  thread_local State state;
  (void)state;
}
#endif
MuMySqlConfig EnvironmentConfig(const char* user, const char* password) {
  MuMySqlConfig config;
  config.host = Env("MU_MYSQL_HOST"); config.database = Env("MU_MYSQL_DATABASE");
  config.user = std::getenv("MU_MYSQL_USER") ? Env("MU_MYSQL_USER") : (user ? user : "");
  config.password = std::getenv("MU_MYSQL_PASSWORD") ? Env("MU_MYSQL_PASSWORD") : (password ? password : "");
  const auto port = Env("MU_MYSQL_PORT");
  if (!port.empty()) {
    if (port.find_first_not_of("0123456789") != std::string::npos) throw std::invalid_argument("invalid MU_MYSQL_PORT");
    const auto value = std::stoul(port);
    if (!value || value > 65535) throw std::invalid_argument("invalid MU_MYSQL_PORT");
    config.port = static_cast<unsigned int>(value);
  }
  if (!Env("MU_MYSQL_SSL_MODE").empty()) config.sslMode = Env("MU_MYSQL_SSL_MODE");
  config.sslCa = Env("MU_MYSQL_SSL_CA");
  return config;
}
} // namespace

struct CQueryManager::Impl {
  std::mutex mutex;
  std::condition_variable changed;
  bool session = false, configured = false, connected = false, badBinding = false;
  std::thread::id owner;
  MuMySqlConfig config;
  std::map<int, Parameter> parameters;
  mu::db::SqlResult result;
  std::int64_t rowCount = -1;
  std::string error;
#if defined(MU_HAVE_MYSQL) && MU_HAVE_MYSQL
  MYSQL* mysql = nullptr;
#endif
  void Wait(std::unique_lock<std::mutex>& lock) { changed.wait(lock, [&] { return !session || owner == std::this_thread::get_id(); }); }
  void Begin(std::unique_lock<std::mutex>& lock) { Wait(lock); session = true; owner = std::this_thread::get_id(); }
  void End() { session = false; owner = {}; changed.notify_all(); }
  void Reset() { result.Clear(); parameters.clear(); rowCount = -1; badBinding = false; }
  void Disconnect() {
#if defined(MU_HAVE_MYSQL) && MU_HAVE_MYSQL
    if (mysql) { EnsureMySqlThread(); mysql_close(mysql); }
    mysql = nullptr;
#endif
    connected = false;
  }
  bool Connect() {
    Disconnect();
    if (config.host.empty() || config.database.empty() || config.user.empty() || !config.port || config.port > 65535) {
      error = "MySQL requires an explicit host, database, user and valid port; a Windows ODBC DSN is not a MySQL endpoint";
      return false;
    }
#if defined(MU_HAVE_MYSQL) && MU_HAVE_MYSQL
    static std::once_flag once;
    static bool initialized = false;
    std::call_once(once, [] { initialized = mysql_library_init(0, nullptr, nullptr) == 0; });
    if (!initialized) { error = "MySQL client library initialization failed"; return false; }
    EnsureMySqlThread();
    mysql = mysql_init(nullptr);
    if (!mysql) { error = "MySQL client allocation failed"; return false; }
    auto option = [&](mysql_option name, const void* value) { return mysql_options(mysql, name, value) == 0; };
    mysql_ssl_mode ssl;
    if (config.sslMode == "disabled") ssl = SSL_MODE_DISABLED;
    else if (config.sslMode == "preferred") ssl = SSL_MODE_PREFERRED;
    else if (config.sslMode == "required") ssl = SSL_MODE_REQUIRED;
    else if (config.sslMode == "verify_ca") ssl = SSL_MODE_VERIFY_CA;
    else if (config.sslMode == "verify_identity") ssl = SSL_MODE_VERIFY_IDENTITY;
    else { error = "unsupported MySQL TLS mode"; Disconnect(); return false; }
    const char* init = "SET SESSION sql_mode=CONCAT_WS(',',@@SESSION.sql_mode,'NO_BACKSLASH_ESCAPES')";
    if (!option(MYSQL_OPT_CONNECT_TIMEOUT, &config.connectTimeoutSeconds) || !option(MYSQL_OPT_READ_TIMEOUT, &config.readTimeoutSeconds) ||
        !option(MYSQL_OPT_WRITE_TIMEOUT, &config.writeTimeoutSeconds) || !option(MYSQL_SET_CHARSET_NAME, config.charset.c_str()) ||
        !option(MYSQL_OPT_SSL_MODE, &ssl) || !option(MYSQL_INIT_COMMAND, init) ||
        (!config.sslCa.empty() && !option(MYSQL_OPT_SSL_CA, config.sslCa.c_str()))) {
      error = "MySQL client option configuration failed"; Disconnect(); return false;
    }
    if (!mysql_real_connect(mysql, config.host.c_str(), config.user.c_str(), config.password.c_str(), config.database.c_str(), config.port,
                            nullptr, CLIENT_MULTI_RESULTS | CLIENT_PS_MULTI_RESULTS)) {
      // Inspect the driver message only internally. The helper emits fixed TLS
      // categories and never exposes raw errors, endpoint names or credentials.
      const unsigned int code = mysql_errno(mysql);
      const char* privateMessage = code == CR_SSL_CONNECTION_ERROR ? mysql_error(mysql) : nullptr;
      error = mu::db::SanitizedMySqlConnectError(code, privateMessage ? privateMessage : "");
      Disconnect(); return false;
    }
    connected = true; error.clear(); return true;
#else
    error = "native QueryManager was built without libmysqlclient; install/link the actual MySQL C API dependency";
    return false;
#endif
  }
#if defined(MU_HAVE_MYSQL) && MU_HAVE_MYSQL
  bool Execute(const mu::db::TranslatedQuery& query) {
    if (!connected && !Connect()) return false;
    EnsureMySqlThread();
    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    if (!stmt) { error = "MySQL statement allocation failed"; return false; }
    auto failed = [&] {
      const auto code = mysql_stmt_errno(stmt);
      error = "MySQL query failed (client code " + std::to_string(code) + ")";
      mysql_stmt_close(stmt);
      if (code == CR_SERVER_GONE_ERROR || code == CR_SERVER_LOST) Disconnect();
      return false;
    };
    if (mysql_stmt_prepare(stmt, query.sql.c_str(), static_cast<unsigned long>(query.sql.size()))) return failed();
    const auto count = mysql_stmt_param_count(stmt);
    if (count != query.placeholders || parameters.size() != count) {
      error = "bound parameter count does not match SQL placeholders"; mysql_stmt_close(stmt); return false;
    }
    std::vector<MYSQL_BIND> binds(count);
    std::vector<unsigned long> lengths(count);
    for (unsigned long i = 0; i < count; ++i) {
      const auto found = parameters.find(static_cast<int>(i + 1));
      if (found == parameters.end()) { error = "missing one-based SQL parameter"; mysql_stmt_close(stmt); return false; }
      lengths[i] = static_cast<unsigned long>(found->second.bytes.size());
      binds[i].buffer_type = found->second.binary ? MYSQL_TYPE_BLOB : MYSQL_TYPE_STRING;
      binds[i].buffer = found->second.bytes.data();
      binds[i].buffer_length = lengths[i]; binds[i].length = &lengths[i];
    }
    if (count && mysql_stmt_bind_param(stmt, binds.data())) return failed();
    const bool updateMax = true;
    if (mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &updateMax) || mysql_stmt_execute(stmt)) return failed();
    bool selected = false;
    std::size_t totalBytes = 0;
    int next = 0;
    do {
      const auto columns = mysql_stmt_field_count(stmt);
      if (columns > 100) { error = "MySQL result exceeds source MAX_COLUMNS"; mysql_stmt_close(stmt); return false; }
      if (!columns) {
        if (!selected) rowCount = static_cast<std::int64_t>(mysql_stmt_affected_rows(stmt));
      } else {
        if (mysql_stmt_store_result(stmt)) return failed();
        MYSQL_RES* metadata = mysql_stmt_result_metadata(stmt);
        if (!metadata) return failed();
        MYSQL_FIELD* fields = mysql_fetch_fields(metadata);
        if (!selected) {
          selected = true;
          std::vector<MYSQL_BIND> output(columns);
          std::vector<std::vector<unsigned char>> buffers(columns);
          std::vector<unsigned long> actual(columns);
          std::vector<std::uint64_t> bitValues(columns);
          std::vector<bool> bitColumns(columns, false);
          auto nulls = std::make_unique<bool[]>(columns);
          auto truncations = std::make_unique<bool[]>(columns);
          for (unsigned int i = 0; i < columns; ++i) {
            if (fields[i].max_length > 8192) { error = "MySQL result column exceeds source 8192-byte bound"; mysql_free_result(metadata); mysql_stmt_close(stmt); return false; }
            const bool binary = fields[i].charsetnr == 63 && (fields[i].type == MYSQL_TYPE_STRING || fields[i].type == MYSQL_TYPE_VAR_STRING || fields[i].type == MYSQL_TYPE_VARCHAR ||
                fields[i].type == MYSQL_TYPE_TINY_BLOB || fields[i].type == MYSQL_TYPE_MEDIUM_BLOB || fields[i].type == MYSQL_TYPE_LONG_BLOB || fields[i].type == MYSQL_TYPE_BLOB);
            result.columns.push_back({fields[i].name, binary});
            buffers[i].resize(std::max<unsigned long>(1, fields[i].max_length + 1));
            output[i].buffer_type = binary ? MYSQL_TYPE_BLOB : MYSQL_TYPE_STRING;
            output[i].buffer = buffers[i].data(); output[i].buffer_length = static_cast<unsigned long>(buffers[i].size());
            if (fields[i].type == MYSQL_TYPE_BIT) {
              // MySQL BIT's packed bytes are not ASCII digits; ODBC source getters
              // expect numeric text for flags. Request numeric conversion explicitly.
              bitColumns[i] = true;
              output[i].buffer_type = MYSQL_TYPE_LONGLONG;
              output[i].buffer = &bitValues[i]; output[i].buffer_length = sizeof(bitValues[i]); output[i].is_unsigned = true;
            }
            output[i].length = &actual[i]; output[i].is_null = &nulls[i]; output[i].error = &truncations[i];
          }
          if (mysql_stmt_bind_result(stmt, output.data())) { mysql_free_result(metadata); return failed(); }
          int status;
          while ((status = mysql_stmt_fetch(stmt)) != MYSQL_NO_DATA) {
            if (status != 0) { mysql_free_result(metadata); return failed(); }
            std::vector<mu::db::SqlCell> row;
            for (unsigned int i = 0; i < columns; ++i) {
              if (truncations[i] || (!bitColumns[i] && actual[i] > buffers[i].size())) { error = "truncated MySQL result refused"; mysql_free_result(metadata); mysql_stmt_close(stmt); return false; }
              if (!nulls[i]) totalBytes += actual[i];
              if (nulls[i]) row.emplace_back();
              else if (bitColumns[i]) { const auto text = std::to_string(bitValues[i]); row.emplace_back(std::vector<unsigned char>(text.begin(), text.end())); }
              else row.emplace_back(std::vector<unsigned char>(buffers[i].begin(), buffers[i].begin() + actual[i]));
            }
            if (result.rows.size() >= config.maxResultRows || totalBytes > config.maxResultBytes) {
              error = "MySQL materialized result exceeds configured limits"; mysql_free_result(metadata); mysql_stmt_close(stmt); return false;
            }
            result.rows.push_back(std::move(row));
          }
          rowCount = static_cast<std::int64_t>(result.rows.size());
        }
        mysql_free_result(metadata);
        mysql_stmt_free_result(stmt);
      }
      next = mysql_stmt_next_result(stmt);
    } while (next == 0);
    if (next > 0) return failed();
    mysql_stmt_close(stmt);
    error.clear();
    return true;
  }
#endif
};

CQueryManager gQueryManager;
CQueryManager::CQueryManager() : impl_(std::make_unique<Impl>()) {}
CQueryManager::~CQueryManager() { Disconnect(); }
void CQueryManager::Configure(MuMySqlConfig config) {
  std::unique_lock lock(impl_->mutex); impl_->Wait(lock); impl_->Disconnect(); impl_->Reset(); impl_->config = std::move(config); impl_->configured = true; impl_->End();
}
bool CQueryManager::Connect(const char* odbc, const char* user, const char* pass) {
  (void)odbc; // A Windows DSN is deliberately never interpreted as a host/database.
  std::unique_lock lock(impl_->mutex); impl_->Wait(lock); impl_->Reset();
  try {
    if (!impl_->configured) impl_->config = EnvironmentConfig(user, pass);
    // Configure is authoritative, including an explicitly empty password.
    const auto success = impl_->Connect(); impl_->End(); return success;
  } catch (const std::exception&) { impl_->error = "invalid explicit MySQL configuration"; impl_->End(); return false; }
}
bool CQueryManager::GetStatus() { std::lock_guard lock(impl_->mutex); return impl_->connected; }
void CQueryManager::Disconnect() { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); impl_->Disconnect(); impl_->Reset(); impl_->End(); }
void CQueryManager::Diagnostic(const char* query) { (void)query; /* LastError exposes only sanitized status; never print formatted credentials or blobs. */ }
std::string EscapeSqlString(const std::string& value, std::size_t limit) {
  std::string out;
  const std::size_t n = limit ? std::min(limit, value.size()) : value.size();
  out.reserve(n + 8);
  for (std::size_t i = 0; i < n; ++i) {
    const char c = value[i];
    if (c == '\\' || c == '\'') out += '\\';
    out += c;
  }
  return out;
}

std::string FormatSqlQuery(const std::string& format, va_list args) {
  std::string out;
  for (std::size_t i = 0; i < format.size();) {
    if (format[i] != '%') { out += format[i++]; continue; }
    if (i + 1 < format.size() && format[i + 1] == '%') { out += '%'; i += 2; continue; }
    const std::size_t start = i++;
    while (i < format.size() && std::string_view("-+ #0.123456789*").find(format[i]) != std::string_view::npos) ++i;
    if (i >= format.size()) throw std::invalid_argument("invalid SQL format conversion");
    const char conv = format[i++];
    if (conv == 's') {
      const char* value = va_arg(args, const char*);
      std::size_t precision = 0;
      const std::size_t dot = format.find('.', start);
      if (dot != std::string::npos && dot < i) {
        std::size_t p = dot + 1;
        while (p < i && std::isdigit(static_cast<unsigned char>(format[p]))) precision = precision * 10 + (format[p++] - '0');
      }
      out += EscapeSqlString(value ? value : "", precision);
      continue;
    }
    const std::string spec = format.substr(start, i - start);
    char buffer[512]{};
    if (conv == 'd' || conv == 'i') {
      if (spec.find("ll") != std::string::npos) std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, long long));
      else if (spec.find('l') != std::string::npos) std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, long));
      else std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, int));
    } else if (conv == 'u' || conv == 'o' || conv == 'x' || conv == 'X') {
      if (spec.find("ll") != std::string::npos) std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, unsigned long long));
      else if (spec.find('l') != std::string::npos) std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, unsigned long));
      else std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, unsigned int));
    } else if (conv == 'f' || conv == 'e' || conv == 'E' || conv == 'g' || conv == 'G') {
      std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, double));
    } else if (conv == 'c') {
      std::snprintf(buffer, sizeof(buffer), spec.c_str(), va_arg(args, int));
    } else {
      throw std::invalid_argument("unsupported SQL format conversion");
    }
    out += buffer;
  }
  return out;
}

bool CQueryManager::ExecQuery(const char* source, ...) {
  std::unique_lock lock(impl_->mutex); impl_->Begin(lock); impl_->result.Clear(); impl_->rowCount = -1;
  bool success = false;
  try {
    if (impl_->badBinding) throw std::invalid_argument("invalid SQL binding refused");
    if (!source || !mu::db::IsKnownQueryFormat(source)) throw std::invalid_argument("SQL format is outside the original Join/Data query catalog");
    const auto format = mu::db::PortablePrintfFormat(source);
    va_list args; va_start(args, source);
    std::string query;
    try { query = FormatSqlQuery(format, args); } catch (...) { va_end(args); throw; }
    va_end(args);
    if (query.size() >= 4096) throw std::invalid_argument("formatted SQL exceeds source 4096-byte bound");
    const auto translated = mu::db::TranslateQuery(source, query);
#if defined(MU_HAVE_MYSQL) && MU_HAVE_MYSQL
    success = impl_->Execute(translated);
#else
    (void)translated; impl_->error = "native QueryManager was built without libmysqlclient";
#endif
  } catch (const std::exception& error) { impl_->error = error.what(); }
  impl_->parameters.clear();
  impl_->badBinding = false;
  if (!success) { impl_->result.Clear(); impl_->End(); }
  return success;
}
void CQueryManager::Close() { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); impl_->Reset(); impl_->End(); }
SQLRETURN CQueryManager::Fetch() { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); return impl_->result.Fetch() ? kSuccess : kNoData; }
int CQueryManager::FindIndex(const char* column) { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); return impl_->result.Find(column); }
int CQueryManager::GetResult(int index) { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); return impl_->result.Integer(index); }
int CQueryManager::GetAsInteger(const char* column) { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); return impl_->result.Integer(impl_->result.Find(column)); }
float CQueryManager::GetAsFloat(const char* column) { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); return impl_->result.Float(impl_->result.Find(column)); }
std::int64_t CQueryManager::GetAsInteger64(const char* column) { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); return impl_->result.Integer64(impl_->result.Find(column)); }
void CQueryManager::GetAsString(const char* column, char* out, int size) { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); impl_->result.String(impl_->result.Find(column), out, size); }
void CQueryManager::GetAsBinary(const char* column, unsigned char* out, int size) {
  std::unique_lock lock(impl_->mutex); impl_->Wait(lock);
  const int index = impl_->result.Find(column);
  if (index < 0 || impl_->result.columns[index].binary) impl_->result.Binary(index, out, size);
  else { auto text = impl_->result.Text(index); ConvertStringToBinary(text.data(), static_cast<int>(text.size()), out, size); }
}
void CQueryManager::BindParameterAsString(int number, void* buffer, int size) {
  std::unique_lock lock(impl_->mutex); impl_->Begin(lock);
  if (number < 1 || number > 100 || !buffer || size < 0) { impl_->error = "invalid SQL string binding"; impl_->badBinding = true; return; }
  const auto length = strnlen(static_cast<char*>(buffer), static_cast<std::size_t>(size));
  auto* bytes = static_cast<unsigned char*>(buffer); impl_->parameters[number] = {std::vector<unsigned char>(bytes, bytes + length), false};
}
void CQueryManager::BindParameterAsBinary(int number, void* buffer, int size) {
  std::unique_lock lock(impl_->mutex); impl_->Begin(lock);
  if (number < 1 || number > 100 || (!buffer && size) || size < 0) { impl_->error = "invalid SQL binary binding"; impl_->badBinding = true; return; }
  auto* bytes = static_cast<unsigned char*>(buffer); impl_->parameters[number] = {size ? std::vector<unsigned char>(bytes, bytes + size) : std::vector<unsigned char>(), true};
}
void CQueryManager::ConvertStringToBinary(char* input, int inputSize, unsigned char* out, int outSize) {
  if (!out || outSize <= 0) return;
  std::memset(out, 0, static_cast<std::size_t>(outSize));
  if (!input || inputSize <= 0) return;
  const auto length = strnlen(input, static_cast<std::size_t>(inputSize));
  for (std::size_t i = 0; i + 1 < length && i / 2 < static_cast<std::size_t>(outSize); i += 2) {
    const int hi = Hex(input[i]), lo = Hex(input[i + 1]);
    if (hi < 0 || lo < 0) { std::memset(out, 0, static_cast<std::size_t>(outSize)); return; }
    out[i / 2] = static_cast<unsigned char>((hi << 4) | lo);
  }
}
void CQueryManager::ConvertBinaryToString(unsigned char* input, int inputSize, char* out, int outSize) {
  if (!out || outSize <= 0) return;
  std::memset(out, 0, static_cast<std::size_t>(outSize));
  if (!input || inputSize <= 0) return;
  const char* hex = "0123456789ABCDEF";
  const auto size = std::min(static_cast<std::size_t>(inputSize), static_cast<std::size_t>(outSize - 1) / 2);
  for (std::size_t i = 0; i < size; ++i) { out[i * 2] = hex[input[i] >> 4]; out[i * 2 + 1] = hex[input[i] & 15]; }
}
std::int64_t CQueryManager::GetRowCount() { std::unique_lock lock(impl_->mutex); impl_->Wait(lock); return impl_->rowCount; }
std::string CQueryManager::LastError() { std::lock_guard lock(impl_->mutex); return impl_->error; }
