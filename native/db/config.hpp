#pragma once
#include "query_manager.hpp"
#include "mu/platform.hpp"
namespace mu {
inline unsigned native_port(const std::string& value,const char* label){
 if(value.empty()||value.find_first_not_of("0123456789")!=std::string::npos)
  throw std::runtime_error(std::string(label)+" must be 1..65535");
 const auto number=std::stoul(value);
 if(number<1||number>65535)throw std::runtime_error(std::string(label)+" must be 1..65535");
 return static_cast<unsigned>(number);
}
inline MuMySqlConfig mysql_config(const std::string& file){
 const auto setting=[&](const char* env,const char* key,const char* fallback=""){
  if(const char* value=std::getenv(env))return std::string(value);
  char value[4096]{};GetPrivateProfileString("MySQL",key,fallback,value,sizeof(value),file.c_str());return std::string(value);
 };
 MuMySqlConfig config;
 config.host=setting("MU_MYSQL_HOST","Host");config.database=setting("MU_MYSQL_DATABASE","Database");
 config.user=setting("MU_MYSQL_USER","User");config.password=setting("MU_MYSQL_PASSWORD","Password");
 config.port=native_port(setting("MU_MYSQL_PORT","Port","3306"),"MySQL port");
 config.sslMode=setting("MU_MYSQL_SSL_MODE","SslMode","required");config.sslCa=setting("MU_MYSQL_SSL_CA","SslCa");
 if(config.host.empty()||config.database.empty()||config.user.empty())
  throw std::runtime_error("MySQL requires Host, Database and User in --mysql-config or MU_MYSQL_HOST/DATABASE/USER.");
 if(config.sslMode!="disabled"&&config.sslMode!="preferred"&&config.sslMode!="required"&&config.sslMode!="verify_ca"&&config.sslMode!="verify_identity")
  throw std::runtime_error("Invalid MySQL SslMode");
 return config;
}
}
