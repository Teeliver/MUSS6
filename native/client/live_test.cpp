// Explicit opt-in integration test. Uses a private test-account credential file.
#include "client.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>
static void require(bool ok,const char* error){if(!ok)throw std::runtime_error(error);}
int main(int argc,char** argv){
 try{
  require(argc==2,"Usage: MuClient-live-test PRIVATE_TEST_CREDENTIALS");
  struct stat info{};require(stat(argv[1],&info)==0&&S_ISREG(info.st_mode)&&info.st_uid==getuid()&&(info.st_mode&077)==0,"Credentials must be an owner-only file");
  std::ifstream file(argv[1]);std::string account,password;std::getline(file,account);std::getline(file,password);
  require(account=="muverify","Use the isolated muverify test account");
  auto profile=mu::client::load_profile();
  {mu::client::Session s(profile,0);require(s.login(account,"WrongPass!")==0,"Wrong-password rejection failed");}
  {mu::client::Session s(profile,0);require(s.login("NoSuchTest",password)==2,"Missing-account rejection failed");}
  {auto mismatched=profile;mismatched.serial[0]=mismatched.serial[0]=='A'?'B':'A';mu::client::Session s(mismatched,0);require(s.login(account,password)==6,"Serial mismatch rejection failed");}
  {mu::client::Session primary(profile,0);require(primary.login(account,password)==1,"Valid login failed");auto list=primary.characters();require(!list.characters.empty(),"Test account needs a real character");
   mu::client::Session second(profile,0);require(second.login(account,password)==3,"Duplicate-login rejection failed");
   primary.keepalive();primary.disconnect();}
  bool reconnected=false;
  for(unsigned attempt=0;attempt<10&&!reconnected;attempt++){
   std::this_thread::sleep_for(std::chrono::milliseconds(150));mu::client::Session s(profile,0);auto result=s.login(account,password);if(result==1){s.characters();reconnected=true;}else require(result==3,"Unexpected reconnect response");
  }
  require(reconnected,"Disconnected account remained locked online");std::fill(password.begin(),password.end(),0);
  std::cout<<"PASS: wrong password, missing account, serial mismatch, valid login/list, duplicate login, encrypted keepalive, reconnect.\n";
  return 0;
 }catch(const std::exception& e){std::cerr<<"Live test: "<<e.what()<<"\n";return 1;}
}
