// Opt-in integration check; no character creation, deletion or movement.
#include "client.hpp"
#include "character_resources.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
void need(bool ok,const char* s){if(!ok)throw std::runtime_error(s);}
int main(int argc,char** argv){try{
 need(argc==3,"Usage: MuClient-switch-test PRIVATE_CREDENTIALS CHARACTER");struct stat st{};need(stat(argv[1],&st)==0&&st.st_uid==getuid()&&(st.st_mode&077)==0,"Credentials must be owner-only");std::ifstream f(argv[1]);std::string account,password;std::getline(f,account);std::getline(f,password);need(account=="muverify","Only isolated test account permitted");auto profile=mu::client::load_profile();mu::client::Session s(profile,0);auto login=s.login(account,password);if(login!=1)throw std::runtime_error("Test login result="+std::to_string(login));std::fill(password.begin(),password.end(),0);auto before=s.characters();need(!before.characters.empty(),"No test characters");auto entry=s.select(argv[2]);auto end=std::chrono::steady_clock::now()+std::chrono::seconds(3);while((!s.state().statsReady||!s.state().inventoryReady)&&std::chrono::steady_clock::now()<end){s.pump();std::this_thread::sleep_for(std::chrono::milliseconds(25));}
 unsigned notices=0;s.setNoticeHandler([&](const mu::client::ServerNotice& n){if(n.type==1){notices++;std::cout<<"Server notice: "<<n.message<<std::endl;}});auto state=s.state();need(state.vitals.ready&&state.vitals.maxHp>0&&state.vitals.maxMp>0,"HUD authoritative vitals missing");need(state.inventoryReady,"Server equipment missing");need(state.attributes.ready&&state.attributes.combatReady,"Character attributes missing");std::cout<<"Character points="<<state.attributes.points<<" inventory items="<<state.inventory->items.size()<<std::endl;auto resources=mu::viewer::character_resources(entry.character,state.inventory.get());need(resources.baseClass==entry.baseClass,"Character resource class mismatch");auto body=mu::viewer::load_character_body(profile.assetDirectory,resources);need(!body->meshes.empty(),"Selected character model missing");std::cout<<"Selected class="<<resources.baseClass<<" body="<<resources.models[1]<<std::endl;
 std::cout<<"Entered map="<<entry.map<<"; requesting character selection"<<std::endl;s.requestLogout(1);need(notices>=1,"Logout countdown notices were not observed");std::cout<<"Character-selection ACK received"<<std::endl;auto after=s.characters();need(after.characters.size()==before.characters.size(),"Character list changed during switch");std::cout<<"Same session list count="<<after.characters.size()<<std::endl;
 auto second=s.select(argv[2]);std::cout<<"Re-entered map="<<second.map<<std::endl;end=std::chrono::steady_clock::now()+std::chrono::seconds(3);while((!s.state().statsReady||!s.state().inventoryReady)&&std::chrono::steady_clock::now()<end){s.pump();std::this_thread::sleep_for(std::chrono::milliseconds(25));}s.requestLogout(2);std::cout<<"PASS: world -> characters -> world on same socket, then server-selection ACK.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"Switch test: "<<e.what()<<"\n";return 1;}}
