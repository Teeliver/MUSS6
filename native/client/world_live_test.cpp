#include "client.hpp"
#include "assets.hpp"
#include "pathfinding.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <set>
#include <algorithm>
void need(bool ok,const char* s){if(!ok)throw std::runtime_error(s);}
int main(int argc,char** argv){try{
 need(argc==2,"Usage: MuClient-world-test PRIVATE_CREDENTIALS");struct stat st{};need(stat(argv[1],&st)==0&&st.st_uid==getuid()&&(st.st_mode&077)==0,"Credential file must be owner-only");
 std::ifstream f(argv[1]);std::string account,password;std::getline(f,account);std::getline(f,password);need(account=="muverify","Only use the isolated test account");
 auto p=mu::client::load_profile();auto terrain=mu::assets::load_terrain(std::filesystem::path(p.assetDirectory)/"Data",0);mu::client::Session s(p,0);need(s.login(account,password)==1,"Test account login failed");auto entry=s.select("MacGuiDW");
 auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);auto state=s.state();while(!state.statsReady&&std::chrono::steady_clock::now()<until){state=s.pump();std::this_thread::sleep_for(std::chrono::milliseconds(25));}need(state.statsReady,"Authoritative attack speeds were not received");
 constexpr int dx[]={-1,0,1,1,1,0,-1,-1},dy[]={-1,-1,-1,0,1,1,1,0};unsigned dir=8;
 for(unsigned d=0;d<8;d++){int x=int(state.x)+dx[d],y=int(state.y)+dy[d];if(x>0&&y>0&&x<255&&y<255&&(terrain.attributes[y*256+x]&12)==0){dir=d;break;}}
 need(dir<8,"No neighboring walkable tile");unsigned startX=state.x,startY=state.y;state=s.movePath(startX,startY,{dir,(dir+4)%8,dir});need(state.x==unsigned(int(startX)+dx[dir])&&state.y==unsigned(int(startY)+dy[dir]),"Movement was not confirmed by server");
 // Visit the actual warehouse vicinity from this runtime's Lorencia spawn data.
 auto pause=[&](double seconds){auto end=std::chrono::steady_clock::now()+std::chrono::duration<double>(seconds);while(std::chrono::steady_clock::now()<end){state=s.pump();std::this_thread::sleep_for(std::chrono::milliseconds(25));}};pause(1.8);
 auto route=mu::viewer::find_path(terrain.attributes,state.x,state.y,146,113);need(!route.empty()||(state.x==146&&state.y==113),"No test route to warehouse vicinity");
 for(std::size_t i=0;i<route.size();i+=14){auto finish=std::min(route.size(),i+14);std::vector<unsigned> steps(route.begin()+i,route.begin()+finish);state=s.movePath(state.x,state.y,steps);pause(steps.size()*.52+.25);}
 pause(1);unsigned npcIndex=0;bool npcFound=false;for(auto& pair:state.viewport->entities)if(pair.second.type==543||pair.second.type==240){npcIndex=pair.first;npcFound=true;break;}if(!npcFound){std::cerr<<"Viewport at "<<state.x<<","<<state.y<<":";for(auto& pair:state.viewport->entities)std::cerr<<pair.second.type<<",";throw std::runtime_error("Test NPC missing from server viewport");}s.talk(npcIndex);pause(2);need(state.npc.replied&&(state.npc.result==2||(state.npc.result==0xF9&&state.npc.npcClass==543)),"Encrypted NPC talk did not open server interface");s.closeTalk();pause(1);auto beforeCloseX=state.x,beforeCloseY=state.y;state=s.movePath(state.x,state.y,{dir,(dir+4)%8});need(state.x==beforeCloseX&&state.y==beforeCloseY,"Movement after NPC close failed");pause(1.5);std::cout<<"PASS: encrypted NPC talk and close. ";
 std::size_t maxEntities=0;std::set<unsigned> types;auto stop=std::chrono::steady_clock::now()+std::chrono::seconds(8);while(std::chrono::steady_clock::now()<stop){state=s.pump();if(state.viewport){maxEntities=std::max(maxEntities,state.viewport->entities.size());for(auto& pair:state.viewport->entities)types.insert(pair.second.type);}std::this_thread::sleep_for(std::chrono::milliseconds(25));}
 need(maxEntities>0,"No authoritative viewport entities received near test character");s.disconnect();std::cout<<"Viewport max="<<maxEntities<<" types=";for(auto type:types)std::cout<<type<<",";std::cout<<". ";std::cout<<"PASS: persistent world session, server stats, three-step packed path confirmation, encrypted heartbeat; moved "<<startX<<","<<startY<<" -> "<<state.x<<","<<state.y<<"; received="<<state.packets<<". Verify saved position in MySQL.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"World test: "<<e.what()<<"\n";return 1;}}
