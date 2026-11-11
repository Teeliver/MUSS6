#pragma once

#include <initializer_list>
#include <string>
#include <string_view>

namespace mu::db {

// CR_SSL_CONNECTION_ERROR in the MySQL C API. This helper has no driver/socket
// dependencies so private driver messages can be classified in offline tests.
inline constexpr unsigned int kMySqlTlsClientError = 2026;
enum class MySqlTlsFailure {
  NotTls,
  ServerLacksTls,
  CertificateVerification,
  ProtocolMismatch,
  Unclassified
};

inline MySqlTlsFailure ClassifyMySqlTlsError(unsigned int code, std::string_view privateMessage) {
  if (code != kMySqlTlsClientError) return MySqlTlsFailure::NotTls;
  std::string lower;
  lower.reserve(privateMessage.size());
  for (const unsigned char byte : privateMessage)
    lower.push_back(static_cast<char>(byte >= 'A' && byte <= 'Z' ? byte + ('a' - 'A') : byte));
  const auto containsAny = [&](std::initializer_list<std::string_view> markers) {
    for (const auto marker : markers)
      if (lower.find(marker) != std::string::npos) return true;
    return false;
  };
  // Match concrete English MySQL/OpenSSL diagnostics rather than inferring a
  // cause from a broad word such as "SSL" or from the configured TLS mode.
  if (containsAny({
      "ssl is required but the server doesn't support it",
      "ssl is required but the server does not support it",
      "ssl is required, but the server does not support it",
      "ssl is required, but the server doesn't support it",
      "ssl is required but server does not support it",
      "tls is required but the server doesn't support it",
      "tls is required but the server does not support it"}))
    return MySqlTlsFailure::ServerLacksTls;
  if (containsAny({
      "certificate verify failed", "certificate verify failure",
      "certificate verification failed", "certificate verification failure",
      "certificate validation failed", "certificate validation failure",
      "validation of ssl server certificate failed",
      "unable to get local issuer certificate", "unable to verify the first certificate",
      "self-signed certificate", "self signed certificate",
      "certificate has expired", "certificate is not yet valid",
      "hostname mismatch", "host name mismatch", "ip address mismatch"}))
    return MySqlTlsFailure::CertificateVerification;
  if (containsAny({
      "wrong version number", "unsupported protocol", "unknown protocol",
      "alert protocol version", "protocol version mismatch",
      "no protocols available", "unsupported ssl version", "unsupported tls version",
      "version too low", "version too high", "no shared cipher", "no ciphers available"}))
    return MySqlTlsFailure::ProtocolMismatch;
  return MySqlTlsFailure::Unclassified;
}

// Return only fixed category text plus the already public numeric client code.
// Never concatenate any portion of privateMessage into diagnostics or logs.
inline std::string SanitizedMySqlConnectError(unsigned int code, std::string_view privateMessage) {
  const std::string prefix = "MySQL connection failed (client code " + std::to_string(code) + ")";
  switch (ClassifyMySqlTlsError(code, privateMessage)) {
    case MySqlTlsFailure::ServerLacksTls:
      return prefix + ": TLS is required but the server does not support TLS";
    case MySqlTlsFailure::CertificateVerification:
      return prefix + ": TLS certificate verification failed";
    case MySqlTlsFailure::ProtocolMismatch:
      return prefix + ": TLS protocol or cipher mismatch";
    case MySqlTlsFailure::Unclassified:
      return prefix + ": TLS handshake failed (cause not classified)";
    case MySqlTlsFailure::NotTls:
      return prefix;
  }
  return prefix;
}

} // namespace mu::db
