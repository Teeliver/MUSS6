#if defined(NDEBUG)
#undef NDEBUG
#endif
#include "query_manager.hpp"
#include "sql_dialect.hpp"
#include "sql_result.hpp"
#include "known_queries.hpp"
#include "tls_error.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace {
std::string FixtureFormat(std::string_view format) {
  std::string result;
  for (std::size_t i = 0; i < format.size(); ++i) {
    if (format[i] != '%') { result += format[i]; continue; }
    if (i + 1 < format.size() && format[i + 1] == '%') { result += '%'; ++i; continue; }
    ++i;
    while (i < format.size() && std::strchr("diufoxXscgeGEp", format[i]) == nullptr) ++i;
    assert(i < format.size());
    result += format[i] == 's' ? "marker_getdate_[Index]_?_" : "3840";
  }
  return result;
}
template<class F> void Reject(F fn) { bool failed = false; try { fn(); } catch (const std::exception&) { failed = true; } assert(failed); }
std::vector<unsigned char> Bytes(const char* value) { return {value, value + std::strlen(value)}; }
}
int main() {
  using namespace mu::db;
  // Synthetic private details must never appear in any public diagnostic. No
  // Connect call is made; these are pure MySQL/OpenSSL message classifications.
  const std::string privateDetails = " host=sensitive-host.example.invalid user=test_user_sensitive password=test_password_sensitive SQL=SELECT private_marker_sensitive";
  struct TlsCase { const char* message; MySqlTlsFailure category; const char* publicMessage; };
  const TlsCase tlsCases[] = {
    {"SSL connection error: SSL is required but the server doesn't support it", MySqlTlsFailure::ServerLacksTls,
      "MySQL connection failed (client code 2026): TLS is required but the server does not support TLS"},
    {"TLS connection error: TLS is required but the server does not support it", MySqlTlsFailure::ServerLacksTls,
      "MySQL connection failed (client code 2026): TLS is required but the server does not support TLS"},
    {"error:0A000086:SSL routines::certificate verify failed", MySqlTlsFailure::CertificateVerification,
      "MySQL connection failed (client code 2026): TLS certificate verification failed"},
    {"SSL connection error: Unable to get local issuer certificate", MySqlTlsFailure::CertificateVerification,
      "MySQL connection failed (client code 2026): TLS certificate verification failed"},
    {"SSL connection error: Validation of SSL server certificate failed", MySqlTlsFailure::CertificateVerification,
      "MySQL connection failed (client code 2026): TLS certificate verification failed"},
    {"SSL connection error: self-signed certificate in certificate chain", MySqlTlsFailure::CertificateVerification,
      "MySQL connection failed (client code 2026): TLS certificate verification failed"},
    {"SSL connection error: Hostname mismatch", MySqlTlsFailure::CertificateVerification,
      "MySQL connection failed (client code 2026): TLS certificate verification failed"},
    {"SSL connection error: CERTIFICATE HAS EXPIRED", MySqlTlsFailure::CertificateVerification,
      "MySQL connection failed (client code 2026): TLS certificate verification failed"},
    {"error:0A00010B:SSL routines::wrong version number", MySqlTlsFailure::ProtocolMismatch,
      "MySQL connection failed (client code 2026): TLS protocol or cipher mismatch"},
    {"error:0A000102:SSL routines::unsupported protocol", MySqlTlsFailure::ProtocolMismatch,
      "MySQL connection failed (client code 2026): TLS protocol or cipher mismatch"},
    {"SSL connection error: tlsv1 alert protocol version", MySqlTlsFailure::ProtocolMismatch,
      "MySQL connection failed (client code 2026): TLS protocol or cipher mismatch"},
    {"SSL connection error: no shared cipher", MySqlTlsFailure::ProtocolMismatch,
      "MySQL connection failed (client code 2026): TLS protocol or cipher mismatch"},
    {"SSL connection error: unexpected EOF", MySqlTlsFailure::Unclassified,
      "MySQL connection failed (client code 2026): TLS handshake failed (cause not classified)"},
    {"SSL connection error: sslv3 alert handshake failure", MySqlTlsFailure::Unclassified,
      "MySQL connection failed (client code 2026): TLS handshake failed (cause not classified)"},
    {"", MySqlTlsFailure::Unclassified,
      "MySQL connection failed (client code 2026): TLS handshake failed (cause not classified)"},
  };
  for (const auto& fixture : tlsCases) {
    const auto raw = std::string(fixture.message) + privateDetails;
    assert(ClassifyMySqlTlsError(2026, raw) == fixture.category);
    const auto publicMessage = SanitizedMySqlConnectError(2026, raw);
    assert(publicMessage == fixture.publicMessage);
    for (const char* marker : {"sensitive-host", "test_user_sensitive", "test_password_sensitive", "private_marker_sensitive"})
      assert(publicMessage.find(marker) == std::string::npos);
  }
  assert(ClassifyMySqlTlsError(2002, "certificate verify failed" + privateDetails) == MySqlTlsFailure::NotTls);
  assert(SanitizedMySqlConnectError(2002, "certificate verify failed" + privateDetails) == "MySQL connection failed (client code 2002)");
  assert(SanitizedMySqlConnectError(1045, "Access denied" + privateDetails) == "MySQL connection failed (client code 1045)");
  assert(IsKnownQueryFormat("EXEC WZ_GetAccountLevel '%s'"));
  assert(!IsKnownQueryFormat("EXEC invented_proc '%s'"));
  assert(PortablePrintfFormat("value=%I64d,%%I64d,%d") == "value=%lld,%%I64d,%d");
  Reject([] { PortablePrintfFormat("%n"); });
  auto proc = TranslateQuery("EXEC WZ_CONNECT_MEMB '%s','%s','%s'", "EXEC WZ_CONNECT_MEMB 'O''Brien','getdate()[Index]','127.0.0.1'");
  assert(proc.sql == "CALL `WZ_CONNECT_MEMB`('O''Brien' , 'getdate()[Index]' , '127.0.0.1')");
  auto mail = TranslateQuery("WZ_WriteMail '%s',?,?,%d,%d", "WZ_WriteMail 'A',?,?,1,2");
  assert(mail.placeholders == 2 && mail.sql.rfind("CALL `WZ_WriteMail`(", 0) == 0);
  Reject([] { TranslateQuery("EXEC WZ_GetAccountLevel '%s'", "EXEC WZ_GetAccountLevel 'A','B'"); });
  Reject([] { TranslateQuery("EXEC WZ_GetAccountLevel '%s'", "EXEC WZ_GetAccountLevel 'A';DROP TABLE Character"); });
  Reject([] { TranslateQuery("EXEC WZ_GetAccountLevel '%s'", "EXEC WZ_GetAccountLevel 'A'--"); });
  Reject([] { TranslateQuery("%s", "SELECT * FROM Character"); });
  const char* warehouse = "UPDATE warehouse SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) WHERE AccountID='%s'";
  auto blob = TranslateQuery(warehouse, "UPDATE warehouse SET Items=CONVERT(varbinary(3840),REPLICATE(char(0xFF),3840)) WHERE AccountID='A'");
  assert(blob.sql.find("UNHEX(REPEAT('FF',3840))") != std::string::npos);
  assert(blob.sql.find("`warehouse`") != std::string::npos);
  Reject([&] { TranslateQuery(warehouse, "UPDATE warehouse SET Items=CONVERT(varbinary(3840),REPLICATE(char(0xFF),3839)) WHERE AccountID='A'"); });
  for (const char* format : kKnownFormats) {
    auto query = TranslateQuery(format, FixtureFormat(format));
    assert(!query.sql.empty());
    assert(query.sql.find("marker_getdate_[Index]_?_") != std::string::npos || std::string_view(format).find("%s") == std::string_view::npos);
  }
  const char* ranking = "SELECT TOP 6 ROW_NUMBER() OVER(ORDER BY MarkCount DESC) AS Ranking,* FROM ArcaBattleGuildEmblem";
  auto ranked = TranslateQuery(ranking, ranking);
  assert(ranked.sql.find("`ArcaBattleGuildEmblem`.*") != std::string::npos);
  assert(ranked.sql.find("LIMIT 6") != std::string::npos);

  SqlResult result;
  result.columns = {{"Amount",false},{"Life",false},{"Hash",true},{"Missing",false}};
  std::vector<unsigned char> digest = {0,255,1,128,127,4,5,6,7,8,9,10,11,12,13,14};
  result.rows = {{Bytes("4294967295"),Bytes("10.5"),digest,std::nullopt},{Bytes("42"),Bytes("1.25"),std::vector<unsigned char>{0,1},std::nullopt}};
  assert(result.Fetch());
  assert(result.Find("aMoUnT") == 0 && result.Find("unknown") == -1);
  assert(result.Integer(0) == -1 && result.Integer64(0) == 4294967295LL);
  assert(result.Float(1) == 10.5f);
  unsigned char out[20]; result.Binary(2,out,sizeof(out));
  assert(std::memcmp(out,digest.data(),16) == 0 && out[16] == 0);
  char text[4]; result.String(0,text,sizeof(text)); assert(std::strcmp(text,"429") == 0);
  result.String(3,text,sizeof(text)); assert(text[0] == 0);
  assert(result.Fetch()); result.Binary(2,out,sizeof(out)); assert(out[0] == 0 && out[1] == 1 && out[2] == 0);
  assert(!result.Fetch()); assert(result.Integer(-1) == -1);

  CQueryManager manager;
  assert(!manager.GetStatus());
  assert(!manager.ExecQuery("%s", "arbitrary SQL")); // Rejects before configuration/connection, no sockets.
  assert(manager.LastError().find("catalog") != std::string::npos);
  assert(manager.Fetch() == 100);
  manager.BindParameterAsBinary(0,nullptr,1);
  assert(!manager.ExecQuery("SELECT ItemCount FROM GameServerInfo WHERE Number=0"));
  assert(manager.LastError().find("binding") != std::string::npos);
  unsigned char bytes[4] = {0,255,128,7}, decoded[4]; char encoded[9];
  manager.ConvertBinaryToString(bytes,4,encoded,sizeof(encoded)); assert(std::strcmp(encoded,"00FF8007") == 0);
  manager.ConvertStringToBinary(encoded,8,decoded,4); assert(std::memcmp(bytes,decoded,4) == 0);
  char malformed[]="ZZFF"; manager.ConvertStringToBinary(malformed,4,decoded,4); assert(decoded[0] == 0 && decoded[1] == 0);
  manager.Close(); manager.Disconnect();
  std::cout << "native MySQL unit tests passed: TLS error redaction, 271 source query formats, 57 procedure contracts, dialect/result/binary bounds; no DB connection\n";
}
