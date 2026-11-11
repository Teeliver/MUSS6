#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>

// Native return codes retain the ODBC caller contract without requiring unixODBC.
using SQLRETURN = std::int16_t;

struct MuMySqlConfig {
  std::string host, database, user, password;
  unsigned int port = 3306;
  std::string charset = "latin1";
  std::string sslMode = "required"; // disabled, preferred, required, verify_ca, verify_identity
  std::string sslCa;
  unsigned int connectTimeoutSeconds = 10;
  unsigned int readTimeoutSeconds = 15;
  unsigned int writeTimeoutSeconds = 15;
  std::size_t maxResultRows = 100000;
  std::size_t maxResultBytes = 64 * 1024 * 1024;
};

class CQueryManager {
public:
  CQueryManager();
  virtual ~CQueryManager();
  CQueryManager(const CQueryManager&) = delete;
  CQueryManager& operator=(const CQueryManager&) = delete;
  void Configure(MuMySqlConfig config);
  bool Connect(const char* odbc, const char* user, const char* pass);
  bool GetStatus();
  void Disconnect();
  void Diagnostic(const char* query);
  bool ExecQuery(const char* query, ...);
  void Close();
  void Clear() { Close(); }
  SQLRETURN Fetch();
  int FindIndex(const char* column);
  int GetResult(int index);
  int GetAsInteger(const char* column);
  float GetAsFloat(const char* column);
  std::int64_t GetAsInteger64(const char* column);
  void GetAsString(const char* column, char* out, int size);
  void GetAsBinary(const char* column, unsigned char* out, int size);
  void BindParameterAsString(int number, void* buffer, int size);
  void BindParameterAsBinary(int number, void* buffer, int size);
  void ConvertStringToBinary(char* input, int inputSize, unsigned char* out, int outSize);
  void ConvertBinaryToString(unsigned char* input, int inputSize, char* out, int outSize);
  std::int64_t GetRowCount();
  std::string LastError();
private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
extern CQueryManager gQueryManager;
