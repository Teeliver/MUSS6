#include "client.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
int main(int argc,char** argv){
 try{
  std::string config,credentials,createName,selectName;int code=-1,characterClass=0;
  for(int i=1;i<argc;i++){
   std::string arg=argv[i];
   if(arg=="--help"){std::cout<<"MuClient-probe [--config FILE] [--connect-code 0..255] [--credentials FILE] [--create NAME --class 0|16|32|48|64|80|96] [--select NAME]\nCredentials: two lines (account, password), owner-only permissions. Create writes a real character.\n";return 0;}
   if((arg=="--credentials"||arg=="--create"||arg=="--select"||arg=="--class")&&i+1<argc){
    std::string value=argv[++i];if(arg=="--credentials")credentials=value;else if(arg=="--create")createName=value;else if(arg=="--select")selectName=value;else {std::size_t end=0;characterClass=std::stoi(value,&end);if(end!=value.size()||characterClass<0||characterClass>96||characterClass%16)throw std::runtime_error("Invalid class");}continue;
   }
   if((arg=="--config"||arg=="--connect-code")&&i+1<argc){
    std::string value=argv[++i];if(arg=="--config")config=value;
    else{std::size_t end=0;code=std::stoi(value,&end);if(end!=value.size()||code<0||code>255)throw std::runtime_error("Server code must be 0..255");}
   }else throw std::runtime_error("Unknown or incomplete option");
  }
  auto profile=mu::client::load_profile(config);auto servers=mu::client::discover(profile);
  std::cout<<"PASS: ConnectServer discovery; available servers="<<servers.size()<<"\n";
  for(auto server:servers)std::cout<<"Server "<<server.code<<"; load="<<server.load<<"%; type="<<server.type<<"\n";
  if(!credentials.empty()){
   if(code<0)throw std::runtime_error("Login requires --connect-code");
   struct stat info{};if(stat(credentials.c_str(),&info)!=0||!S_ISREG(info.st_mode)||(info.st_mode&077)!=0||info.st_uid!=getuid())throw std::runtime_error("Credential file must be owned by you and inaccessible to group/others");
   std::ifstream file(credentials);std::string account,password;std::getline(file,account);std::getline(file,password);file.close();
   mu::client::Session session(profile,unsigned(code));unsigned result=session.login(account,password);std::fill(password.begin(),password.end(),0);
   std::cout<<"Login result="<<result<<"; "<<mu::client::login_result(result)<<"\n";if(result!=1)return 2;
   auto list=session.characters();std::cout<<"PASS: authenticated character list; count="<<list.characters.size()<<"\n";
   if(!createName.empty()){unsigned result=session.create(createName,unsigned(characterClass));std::cout<<"Create result="<<result<<"\n";if(result!=1)return 3;list=session.characters();std::cout<<"PASS: updated character list; count="<<list.characters.size()<<"\n";}
   if(!selectName.empty()){auto world=session.select(selectName);std::cout<<"PASS: selected character; map="<<world.map<<"; position="<<world.x<<","<<world.y<<"; world-entry bytes="<<world.packet.size()<<"\n";}
   session.disconnect();
  }else{
   if(!createName.empty()||!selectName.empty())throw std::runtime_error("Character operations require --credentials");
   if(code>=0)std::cout<<mu::client::connect_game(profile,unsigned(code))<<"\n";
  }
  return 0;
 }catch(const std::exception& e){std::cerr<<"Client: "<<e.what()<<"\n";return 1;}
}
