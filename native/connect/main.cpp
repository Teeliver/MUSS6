#include "stdafx.h"
#include "BlackList.h"
#include "ServerList.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "Util.h"
#include <arpa/inet.h>
#include <csignal>
#include <filesystem>
#include <iostream>
// The SDK dyld.h also declares legacy TRUE/FALSE enumerators, which collide with
// the Windows-compatible scalars. Use its exact C declaration for this API.
extern "C" int _NSGetExecutablePath(char* buffer,std::uint32_t* size);

int MaxConnectionPerIP=0;
int MaxPacketPerSecond=0;
int MaxConnectionIdle=0;
char CustomerName[32]{};
namespace {
volatile std::sig_atomic_t stopping=0;
volatile std::sig_atomic_t reloadRequested=0;
void Signal(int signal) { if(signal==SIGHUP) reloadRequested=1; else stopping=1; }
bool HasConfig(const std::filesystem::path& directory) {
  for(const char* name:{"ConnectServer.ini","ServerList.dat","BlackList.txt"})
    if(!std::filesystem::is_regular_file(directory/name)) return false;
  return true;
}
std::filesystem::path FindDefaultConfig() {
  const auto cwd=std::filesystem::current_path();
  std::vector<std::filesystem::path> candidates{cwd};
  std::uint32_t size=0;
  _NSGetExecutablePath(nullptr,&size);
  if(size) {
    std::vector<char> executable(size);
    if(_NSGetExecutablePath(executable.data(),&size)==0) {
      const auto parent=std::filesystem::weakly_canonical(executable.data()).parent_path();
      candidates.push_back(parent/"config");
      candidates.push_back(parent/"../../config/connect");
    }
  }
  candidates.push_back(cwd/"native/config/connect");
  for(const auto& directory:candidates) if(HasConfig(directory)) return std::filesystem::weakly_canonical(directory);
  throw std::runtime_error("No default configuration found. Pass --config DIR containing ConnectServer.ini, ServerList.dat and BlackList.txt, or place them in config/ beside the executable.");
}
int Port(const char* key,int fallback) {
  const int value=GetPrivateProfileInt("ConnectServerInfo",key,fallback,".\\ConnectServer.ini");
  if(value<1 || value>65535) throw std::runtime_error(std::string(key)+" must be 1..65535");
  return value;
}
void LoadLimits() {
  MaxConnectionPerIP=GetPrivateProfileInt("ConnectServerInfo","MaxConnectionPerIP",0,".\\ConnectServer.ini");
  MaxPacketPerSecond=GetPrivateProfileInt("ConnectServerInfo","MaxPacketPerSecond",0,".\\ConnectServer.ini");
  MaxConnectionIdle=GetPrivateProfileInt("ConnectServerInfo","MaxConnectionIdle",0,".\\ConnectServer.ini");
  if(MaxConnectionPerIP<0 || MaxPacketPerSecond<0 || MaxConnectionIdle<0 || MaxConnectionIdle>2147483) throw std::runtime_error("Connect limits must be nonnegative; lifetime must fit milliseconds");
}
void LoadAssets() {
  LoadLimits();
  char blacklist[]="BlackList.txt",servers[]="ServerList.dat";
  gBlackList.Load(blacklist); gServerList.Load(servers);
}
}
int main(int argc,char** argv) {
  try {
    std::filesystem::path config;
    std::string host="0.0.0.0";
    bool check=false;
    for(int index=1;index<argc;++index) {
      const std::string argument=argv[index];
      if(argument=="--help") { std::cout<<"ConnectServer-native [--config DIR] [--host IPv4] [--check-config]\nDefault: current directory, config/ beside the executable, or native/config/connect in the source checkout.\nEX603ENG: reuses original C++ discovery/registry; no database dependency.\n"; return 0; }
      if(argument=="--check-config") { check=true; continue; }
      if(argument=="--config" || argument=="--host") {
        if(index+1>=argc || std::string(argv[index+1]).rfind("--",0)==0) throw std::runtime_error("Missing value for "+argument);
        if(argument=="--config") config=std::filesystem::absolute(argv[++index]); else host=argv[++index];
        continue;
      }
      throw std::runtime_error("Unknown argument: "+argument);
    }
    if(config.empty()) config=FindDefaultConfig();
    if(!std::filesystem::is_directory(config)) throw std::runtime_error("Config directory does not exist: "+config.string());
    for(const char* name:{"ConnectServer.ini","ServerList.dat","BlackList.txt"}) if(!std::filesystem::is_regular_file(config/name)) throw std::runtime_error("Missing runtime file: "+(config/name).string());
    in_addr address{}; if(inet_pton(AF_INET,host.c_str(),&address)!=1) throw std::runtime_error("--host must be an IPv4 address");
    std::filesystem::current_path(config);
    const int tcpPort=Port("ConnectServerPortTCP",44405),udpPort=Port("ConnectServerPortUDP",55557);
    LoadAssets();
    std::cout<<"EX603ENG native C++ ConnectServer: config="<<config.string()<<" TCP="<<tcpPort<<" UDP="<<udpPort
             <<" MaxConnectionPerIP="<<MaxConnectionPerIP<<" MaxPacketPerSecond="<<MaxPacketPerSecond<<" MaxConnectionIdle="<<MaxConnectionIdle<<'\n';
    if(check) { std::cout<<"Configuration loaded by original C++ loaders. No sockets were opened.\n"; return 0; }
    if(!MaxConnectionPerIP || !MaxPacketPerSecond || !MaxConnectionIdle) std::cerr<<"Zero limits preserve original rejection/timeout behavior; set positive values for usable client connections.\n";
    std::signal(SIGINT,Signal); std::signal(SIGTERM,Signal); std::signal(SIGHUP,Signal); std::signal(SIGPIPE,SIG_IGN);
    gSocketManager.SetBindAddress(host); gSocketManagerUdp.SetBindAddress(host);
    if(!gSocketManager.Start(static_cast<WORD>(tcpPort)) || !gSocketManagerUdp.Start(static_cast<WORD>(udpPort))) throw std::runtime_error("ConnectServer startup failed");
    DWORD lastTick=GetTickCount();
    while(!stopping) {
      gSocketManagerUdp.Poll(); gSocketManager.Poll(50);
      const DWORD tick=GetTickCount();
      if(tick-lastTick>=1000) { gServerList.MainProc(); ConnectServerTimeoutProc(); lastTick=tick; }
      if(reloadRequested) { reloadRequested=0; LoadAssets(); std::cerr<<"Connect config/blacklist/server list reloaded (ports unchanged).\n"; }
    }
    gSocketManagerUdp.Clean(); gSocketManager.Clean();
    return 0;
  } catch(const std::exception& error) {
    std::cerr<<"ConnectServer-native: "<<error.what()<<'\n';
    gSocketManagerUdp.Clean(); gSocketManager.Clean();
    return 1;
  }
}
