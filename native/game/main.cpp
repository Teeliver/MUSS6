#include "stdafx.h"
#include "GameMain.h"
#include "JSProtocol.h"
#include "QueueTimer.h"
#include "ServerInfo.h"
#include "ServerDisplayer.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "PacketManager.h"
#include "QueryManager.h"
#include "User.h"
#include "Util.h"
#include "CustomAttack.h"
#include "CustomStore.h"
#include "HelperOffline.h"
#include "native_cash_callbacks.hpp"
#include "ScriptLoader.h"
#include "../db/config.hpp"
#include "mu/launch_paths.hpp"
#include <filesystem>
#include <csignal>
#include <fcntl.h>
#include <sys/file.h>
#include "asset_paths.hpp"
int gMapShowMonster=0,gMapShowNPC=0,gMapShowPlayer=1;
namespace {
volatile std::sig_atomic_t stopping=0;
void signal_stop(int){stopping=1;}
std::string setting(const char* key){
 char value[4096]{};GetPrivateProfileString("GameServerInfo",key,"",value,sizeof(value),"./Data/GameServerInfo - Common.dat");return value;
}
struct InstanceLock{
 int fd=-1;
 explicit InstanceLock(int code){
  const auto path=std::filesystem::temp_directory_path()/("mu-ex603-"+std::to_string(getuid())+"-"+std::to_string(code)+".lock");
  fd=::open(path.c_str(),O_CREAT|O_RDWR|O_CLOEXEC|O_NOFOLLOW,0600);
  if(fd<0||flock(fd,LOCK_EX|LOCK_NB)!=0){if(fd>=0)::close(fd);fd=-1;throw std::runtime_error("Could not lock GameServer instance for this ServerCode");}
 }
 ~InstanceLock(){if(fd>=0)::close(fd);}
};
bool service_events(CConnection& connection,bool disconnectPlayers=true){
 const auto events=connection.NativePollEvents();
 if(events&CConnection::NativeRead)connection.DataRecv();
 if(events&CConnection::NativeWrite)connection.DataSendEx();
 if(connection.NativeTakeCloseEvent()){
  if(&connection==&gDataServerConnection)mu::cancelAllCashCallbacks();
  connection.Disconnect();if(disconnectPlayers)gObjAllDisconnect();return false;
 }
 return true;
}
bool shutdown_world(bool initialized){
 gQueueTimer.NativeClear();if(initialized)gScriptLoader.NativeShutdown();mu::cancelAllCashCallbacks();bool delivered=true;
 const DWORD started=GetTickCount();
 const auto pending=[](){return gDataServerConnection.NativePendingSendBytes()+gJoinServerConnection.NativePendingSendBytes();};
 const auto pump=[&](){
  if(gDataServerConnection.NativePendingSendBytes()&&!gDataServerConnection.DataSendEx())delivered=false;
  if(gJoinServerConnection.NativePendingSendBytes()&&!gJoinServerConnection.DataSendEx())delivered=false;
  if(!service_events(gDataServerConnection,false))delivered=false;
  if(!service_events(gJoinServerConnection,false))delivered=false;
 };
 if(initialized){
  gCloseServer=1;
  for(int n=OBJECT_START_USER;n<MAX_OBJECT;++n)if(gObj[n].Connected!=OBJECT_OFFLINE&&gObj[n].Type==OBJECT_USER){
   if(!gDataServerConnection.CheckState()||!gJoinServerConnection.CheckState())delivered=false;
   CloseClient(n);gCustomAttack.AttackOfflineClose(&gObj[n]);gCustomStore.OnPShopAlreadyConnected(&gObj[n]);gHelperOffline.HelperOfflineClose(&gObj[n]);
   pump();
   while(pending()>MAX_BUFF_SIZE/4&&GetTickCount()-started<5000){pump();Sleep(1);}
  }
 }
 gSocketManager.Clean();
 while(pending()&&GetTickCount()-started<5000){pump();Sleep(10);}
 if(pending())delivered=false;
 if(!delivered)std::cerr<<"Shutdown could not deliver all outgoing packets; character saves require database verification.\n";
 gJoinServerConnection.Disconnect();gDataServerConnection.Disconnect();gSocketManagerUdp.Clean();gQueryManager.Disconnect();
 return delivered;
}
void timers(){
 const std::pair<int,int> values[]={
  {QUEUE_TIMER_MONSTER,100},{QUEUE_TIMER_MONSTER_MOVE,100},{QUEUE_TIMER_MONSTER_AI,100},
  {QUEUE_TIMER_MONSTER_AI_MOVE,100},{QUEUE_TIMER_EVENT,100},{QUEUE_TIMER_VIEWPORT,1000},
  {QUEUE_TIMER_FIRST,1000},{QUEUE_TIMER_CLOSE,1000},{QUEUE_TIMER_ACCOUNT_LEVEL,60000},
  {QUEUE_TIMER_MATH_AUTHENTICATOR,10000},{QUEUE_TIMER_LICENSE,21600000},{QUEUE_TIMER_LUA_SCRIPT,1000}};
 for(const auto& [index,delay]:values)gQueueTimer.CreateTimer(index,delay,QueueTimerCallback);
}
}
int main(int argc,char** argv){
 bool initialized=false;
 try{
  std::filesystem::path config,mysqlFile;std::string host="127.0.0.1";bool check=false,audit=false;
  for(int i=1;i<argc;++i){
   const std::string arg=argv[i];
   if(arg=="--help"){
    std::cout<<"GameServer-native [--config GameServerDIR] [--host IPv4] [--mysql-config FILE] [--check-config|--audit-assets]\n"
     <<"EX603ENG original C++ game logic; native POSIX transport, Lua5.2 and MySQL.\n"
     <<"--check-config reads startup INI only. --audit-assets additionally checks referenced shared Data paths.\n";return 0;
   }
   if(arg=="--check-config"){check=true;continue;}if(arg=="--audit-assets"){audit=true;continue;}
   if(arg=="--config"||arg=="--host"||arg=="--mysql-config"){
    if(i+1>=argc||std::string(argv[i+1]).rfind("--",0)==0)throw std::runtime_error("Missing value for "+arg);
    const std::string value=argv[++i];if(arg=="--config")config=std::filesystem::absolute(value);else if(arg=="--mysql-config")mysqlFile=std::filesystem::absolute(value);else host=value;continue;
   }
   throw std::runtime_error("Unknown argument: "+arg);
  }
  const auto executableDir=mu::launch::executable_directory();
  if(config.empty()){
   const auto cwd=std::filesystem::current_path();
   if(std::filesystem::is_regular_file(cwd/"Data/GameServerInfo - Common.dat"))config=cwd;
   else{
    for(const auto& pointer:{executableDir/"../../config/game/runtime.ini",cwd/"native/config/game/runtime.ini"}){
     if(!std::filesystem::is_regular_file(pointer))continue;
     char value[4096]{};
     GetPrivateProfileString("GameServer","RuntimeDirectory","",value,sizeof(value),pointer.c_str());
     if(!value[0])throw std::runtime_error("Missing RuntimeDirectory in "+pointer.string());
     const std::filesystem::path path=value;
     config=std::filesystem::absolute(path.is_absolute()?path:pointer.parent_path()/path);
     break;
    }
   }
  }
  if(config.empty()||!std::filesystem::is_directory(config))throw std::runtime_error("Pass --config with the original GameServer runtime directory, or configure native/config/game/runtime.ini");
  if(mysqlFile.empty())mysqlFile=mu::launch::mysql_config(config,executableDir,std::filesystem::current_path());
  for(const char* name:{"Common","Character","Command","Skill","Event","Custom","ChaosMix"}){
   const auto file=config/"Data"/(std::string("GameServerInfo - ")+name+".dat");
   if(!std::filesystem::is_regular_file(file))throw std::runtime_error("Missing runtime file: "+file.string());
  }
  if(!mysqlFile.empty()&&!std::filesystem::is_regular_file(mysqlFile))throw std::runtime_error("--mysql-config file not found");
  in_addr address{};if(inet_pton(AF_INET,host.c_str(),&address)!=1)throw std::runtime_error("--host must be IPv4");
  std::filesystem::current_path(config);
  for(const char* key:{"ServerPort","JoinServerPort","DataServerPort","ConnectServerPort"})mu::native_port(setting(key),key);
  if(setting("ServerName").empty()||setting("ServerSerial").size()!=16||setting("ServerVersion").size()!=7)
   throw std::runtime_error("ServerName, 16-byte ServerSerial and seven-character ServerVersion are required");
  for(const char* key:{"JoinServerAddress","DataServerAddress","ConnectServerAddress"})if(setting(key).empty())throw std::runtime_error(std::string("Missing ")+key);
  gServerInfo.ReadStartupInfo("GameServerInfo","./Data/GameServerInfo - Common.dat");
  if(gServerInfo.m_ServerMaxUserNumber<=0)throw std::runtime_error("ServerMaxUserNumber must be positive");
  if(!mysqlFile.empty())gQueryManager.Configure(mu::mysql_config(mysqlFile.string()));
  std::cout<<"GameServer runtime: "<<std::filesystem::weakly_canonical(config).string()<<".\n";
  std::cout<<"EX603ENG native C++ GameServer: startup INI loaded; port="<<gServerInfo.m_ServerPort<<".\n";
  if(audit){
   std::size_t missing=0,inputs=0;
   for(const auto* name:mu::game_asset_paths){
    const auto path=config.parent_path()/"Data"/name;
    // WriteItemInfo generates this file; it is not an input to startup.
    if(std::string(name)=="Item/Item.bmd"){
     if(!std::filesystem::is_regular_file(path))std::cout<<"Generated output absent (not a startup input): Data/"<<name<<'\n';
     continue;
    }
    ++inputs;
    if(!std::filesystem::is_regular_file(path)){++missing;std::cout<<"Missing Data/"<<name<<'\n';}
   }
   std::cout<<"Referenced input asset audit: "<<missing<<" missing of "<<inputs<<" paths (some depend on server type or feature settings).\n";
   return missing?2:0;
  }
  if(check){std::cout<<"No gameplay/Lua initialization, database connection or listener opened.\n";return 0;}
  InstanceLock lock(gServerInfo.m_ServerCode);
  SetLargeRand();gCloseServer=0;gServerDisplayer.Init(nullptr);
  GameMainInit(nullptr);initialized=true;
  // Source ignored these loader results. Native readiness requires actual EX603 keys.
  if(!gPacketManager.LoadEncryptionKey("../Data/Hack/Enc2.dat")||!gPacketManager.LoadDecryptionKey("../Data/Hack/Dec1.dat"))
   throw std::runtime_error("Missing or invalid EX603 packet encryption keys");
  gJoinServerConnection.NativeSetConnectTimeout(2000);gDataServerConnection.NativeSetConnectTimeout(2000);
  const bool join=JoinServerConnect(0),data=DataServerConnect(0);
  if(!join||!data)std::cerr<<"JoinServer/DataServer connection pending; clients require both services. Reconnect every 10s.\n";
  if(!gSocketManagerUdp.Connect(gServerInfo.m_ConnectServerAddress,WORD(gServerInfo.m_ConnectServerPort)))throw std::runtime_error("ConnectServer heartbeat socket initialization failed");
  gSocketManager.SetBindAddress(host.c_str());if(!gSocketManager.Start(WORD(gServerInfo.m_ServerPort)))throw std::runtime_error("Game listener startup failed");
  timers();std::signal(SIGINT,signal_stop);std::signal(SIGTERM,signal_stop);std::signal(SIGPIPE,SIG_IGN);
  DWORD second=GetTickCount(),twoSeconds=second,reconnect=second;
  std::cout<<"Game world initialized. Listener="<<host<<":"<<gServerInfo.m_ServerPort<<"; Join="<<join<<" Data="<<data<<". Client compatibility remains to be verified.\n";
  while(!stopping){
   service_events(gJoinServerConnection);service_events(gDataServerConnection);
   gSocketManagerUdp.NativePoll(0);
   if(!gSocketManager.NativePoll(10))throw std::runtime_error("Game listener poll failed: "+gSocketManager.NativeLastError());
   gQueueTimer.NativePoll();const DWORD now=GetTickCount();
   if(now-second>=1000){second=now;mu::pruneCashCallbacks();GJServerUserInfoSend();ConnectServerInfoSend();}
   if(now-twoSeconds>=2000){twoSeconds=now;gObjCountProc();}
   if(now-reconnect>=10000){reconnect=now;JoinServerReconnect(nullptr,0);DataServerReconnect(nullptr,0);}
  }
  return shutdown_world(initialized)?0:1;
 }catch(const std::exception& error){
  std::cerr<<"GameServer-native: "<<error.what()<<'\n';
  shutdown_world(initialized);return 1;
 }
}
