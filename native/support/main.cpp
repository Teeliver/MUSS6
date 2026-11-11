#include "stdafx.h"
#include "AllowableIpList.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "Util.h"
#if defined(MU_NATIVE_JOIN)
#include "AccountManager.h"
#include "SocketManagerUdp.h"
BOOL CaseSensitive=FALSE,MD5Encryption=FALSE;
#else
#include "BadSyntax.h"
#include "GuildManager.h"
#endif
#include <filesystem>
#include <csignal>
#include "mu/launch_paths.hpp"
char CustomerName[32]{};
namespace{
 volatile std::sig_atomic_t stopping=0;
 void signal(int){stopping=1;}
 std::string ini(const std::string& file,const char* section,const char* key,const char* fallback=""){
  char value[4096]{};GetPrivateProfileString(section,key,fallback,value,sizeof(value),file.c_str());return value;
 }
 unsigned port(const std::string& value,const char* label){
  if(value.empty()||value.find_first_not_of("0123456789")!=std::string::npos)throw std::runtime_error(std::string(label)+" must be 1..65535");
  const auto number=std::stoul(value);if(!number||number>65535)throw std::runtime_error(std::string(label)+" must be 1..65535");return unsigned(number);
 }
 MuMySqlConfig database(const std::string& file){
  MuMySqlConfig value;
  const auto setting=[&](const char* env,const char* key,const char* fallback=""){const auto* fromEnv=std::getenv(env);return fromEnv?std::string(fromEnv):ini(file,"MySQL",key,fallback);};
  value.host=setting("MU_MYSQL_HOST","Host");value.database=setting("MU_MYSQL_DATABASE","Database");
  value.user=setting("MU_MYSQL_USER","User");value.password=setting("MU_MYSQL_PASSWORD","Password");
  value.port=port(setting("MU_MYSQL_PORT","Port","3306"),"MySQL port");
  value.sslMode=setting("MU_MYSQL_SSL_MODE","SslMode","required");value.sslCa=setting("MU_MYSQL_SSL_CA","SslCa");
  if(value.host.empty()||value.database.empty()||value.user.empty())throw std::runtime_error("MySQL requires Host, Database and User in --mysql-config or MU_MYSQL_HOST/DATABASE/USER environment variables.");
  if(value.sslMode!="disabled"&&value.sslMode!="preferred"&&value.sslMode!="required"&&value.sslMode!="verify_ca"&&value.sslMode!="verify_identity")throw std::runtime_error("Invalid MySQL SslMode");
  return value;
 }
}
int main(int argc,char** argv){
#if defined(MU_NATIVE_JOIN)
 const std::string kind="JoinServer",section="JoinServerInfo",role="join";const unsigned defaultPort=55970;
#else
 const std::string kind="DataServer",section="DataServerInfo",role="data";const unsigned defaultPort=55960;
#endif
 try{
  std::filesystem::path config,mysqlConfig;std::string host="127.0.0.1";bool check=false,checkDatabase=false;
  for(int i=1;i<argc;++i){
   const std::string arg=argv[i];
   if(arg=="--help"){std::cout<<kind<<"-native [--config DIR] [--mysql-config FILE] [--host IPv4] [--check-config|--check-db]\nDefault: service config beside executable or native/config/"<<role<<"; MySQL from a private mysql.ini or MU_MYSQL_* environment.\nEX603ENG original C++ handlers, native sockets and MySQL C API.\n";return 0;}
   if(arg=="--check-config"){check=true;continue;}if(arg=="--check-db"){checkDatabase=true;continue;}
   if(arg=="--config"||arg=="--mysql-config"||arg=="--host"){
    if(i+1>=argc||std::string(argv[i+1]).rfind("--",0)==0)throw std::runtime_error("Missing value for "+arg);
    const std::string value=argv[++i];if(arg=="--config")config=std::filesystem::absolute(value);else if(arg=="--mysql-config")mysqlConfig=std::filesystem::absolute(value);else host=value;continue;
   }
   throw std::runtime_error("Unknown argument: "+arg);
  }
  const auto cwd=std::filesystem::current_path();
  const auto executableDir=mu::launch::executable_directory();
  if(config.empty())config=mu::launch::service_config(kind,role,executableDir,cwd);
  if(!std::filesystem::is_directory(config))throw std::runtime_error("--config requires the service runtime directory.");
  for(const auto& name:{kind+".ini",std::string("AllowableIpList.txt")})if(!std::filesystem::is_regular_file(config/name))throw std::runtime_error("Missing runtime file: "+(config/name).string());
#if !defined(MU_NATIVE_JOIN)
  if(!std::filesystem::is_regular_file(config/"BadSyntax.txt"))throw std::runtime_error("Missing BadSyntax.txt");
#endif
  if(!mu::launch::has_role(config,kind))throw std::runtime_error(kind+".ini must contain ["+section+"] "+kind+"Port.");
  if(mysqlConfig.empty())mysqlConfig=mu::launch::mysql_config(config,executableDir,cwd);
  if(!mysqlConfig.empty()&&!std::filesystem::is_regular_file(mysqlConfig))throw std::runtime_error("--mysql-config file not found");
  in_addr address{};if(inet_pton(AF_INET,host.c_str(),&address)!=1)throw std::runtime_error("--host must be IPv4");
  std::filesystem::current_path(config);
  const auto listenPort=port(ini(kind+".ini",section.c_str(),(kind+"Port").c_str(),std::to_string(defaultPort).c_str()),"Service port");
  gAllowableIpList.Load(const_cast<char*>("AllowableIpList.txt"));
#if defined(MU_NATIVE_JOIN)
  CaseSensitive=GetPrivateProfileInt(section.c_str(),"CaseSensitive",0,"JoinServer.ini")!=0;
  MD5Encryption=GetPrivateProfileInt(section.c_str(),"MD5Encryption",0,"JoinServer.ini")!=0;
#else
  gBadSyntax.Load(const_cast<char*>("BadSyntax.txt"));
#endif
  const auto mysql=database(mysqlConfig.string());gQueryManager.Configure(mysql);
  std::cout<<kind<<" EX603ENG native C++: config="<<config.string()<<", port="<<listenPort<<".\n";
  std::cout<<"MySQL settings: "<<(mysqlConfig.empty()?"environment variables":mysqlConfig.string())<<" (MU_MYSQL_* overrides file settings).\n";
  if(check){std::cout<<"No database connection or listener opened.\n";return 0;}
  if(!gQueryManager.Connect("", "", ""))throw std::runtime_error("MySQL connection failed: "+gQueryManager.LastError());
  if(checkDatabase){std::cout<<"Connected to MySQL. Schema/procedure behavior has not been validated. No listener opened.\n";gQueryManager.Disconnect();return 0;}
#if defined(MU_NATIVE_JOIN)
  std::string connectHost=ini("JoinServer.ini",section.c_str(),"ConnectServerAddress","127.0.0.1");
  const auto connectPort=port(ini("JoinServer.ini",section.c_str(),"ConnectServerPort","55557"),"ConnectServer UDP port");
  if(!gSocketManagerUdp.Connect(connectHost.data(),WORD(connectPort)))throw std::runtime_error("Could not initialize Join heartbeat socket");
#else
  gGuildManager.Init();
#endif
  gSocketManager.SetBindAddress(host);if(!gSocketManager.Start(WORD(listenPort)))throw std::runtime_error("Could not start listener: "+std::string(std::strerror(errno)));
  std::cout<<"Native "<<kind<<" TCP listening at "<<host<<":"<<listenPort<<"; MySQL connected. Schema/procedure behavior still requires validation.\n";
  std::signal(SIGINT,signal);std::signal(SIGTERM,signal);std::signal(SIGPIPE,SIG_IGN);
  DWORD last=GetTickCount();
  while(!stopping){
   gSocketManager.Poll(50);const DWORD now=GetTickCount();
   if(now-last>=1000){last=now;
#if defined(MU_NATIVE_JOIN)
    JoinServerLiveProc();gAccountManager.DisconnectProc();
#endif
   }
  }
  gSocketManager.Clean();
#if defined(MU_NATIVE_JOIN)
  gSocketManagerUdp.Clean();
#endif
  gQueryManager.Disconnect();return 0;
 }catch(const std::exception& error){std::cerr<<kind<<"-native: "<<error.what()<<'\n';gSocketManager.Clean();gQueryManager.Disconnect();return 1;}
}
