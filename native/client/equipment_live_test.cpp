#include "client.hpp"
#include "item_catalog.hpp"
#include "item_placement.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>
using namespace mu::client;
namespace {
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void ready(Session& s){auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);while(std::chrono::steady_clock::now()<until){if(s.pump().inventoryReady)return;std::this_thread::sleep_for(std::chrono::milliseconds(25));}throw std::runtime_error("Inventory timeout");}
}
int main(int argc,char** argv){try{
 need(argc==2,"Usage: MuClient-equipment-test PRIVATE_CREDENTIALS");struct stat st{};need(stat(argv[1],&st)==0&&st.st_uid==getuid()&&(st.st_mode&077)==0,"Credentials must be owner-only");std::ifstream file(argv[1]);std::string account,password;std::getline(file,account);std::getline(file,password);need(account=="muverify","Only isolated test account permitted");auto profile=load_profile();auto catalog=mu::ui::load_item_catalog(std::filesystem::path(profile.assetDirectory)/"Data",profile.language);
 Session s(profile,0);need(s.login(account,password)==1,"Test account login failed");s.select("MuOptions");ready(s);auto before=*s.state().inventory;std::vector<std::pair<unsigned,unsigned>> candidates;
 for(auto& entry:before.items)if(entry.first<12){for(unsigned slot=12;slot<76;slot++)if(mu::ui::can_place_item(before.items,*catalog,0,slot,entry.second)){candidates.emplace_back(entry.first,slot);break;}}
 for(auto& entry:before.items)if(entry.first>=12&&entry.first<76){auto slot=catalog->at(entry.second.index).wearSlot;if(slot<12&&mu::ui::can_place_item(before.items,*catalog,0,slot,entry.second,int(entry.first)))candidates.emplace_back(entry.first,slot);}
 unsigned source=76,target=76,equipped=12;Item original;bool initiallyEquipped=false;
 for(auto candidate:candidates){auto item=before.items.at(candidate.first);if(!s.moveItem({0,candidate.first,0,candidate.second})){need(s.state().inventory->items.at(candidate.first).wire==item.wire,"Rejected move changed original item");continue;}source=candidate.first;target=candidate.second;original=item;initiallyEquipped=source<12;equipped=initiallyEquipped?source:target;break;}
 need(source<76,"No wearable fixture accepted by server; rejected items retained original positions");std::cout<<"Test item: source slot="<<source<<" temporary destination="<<target<<std::endl;
 auto moved=s.state();need(moved.inventory->items.at(target).wire==original.wire&&!moved.inventory->items.count(source),"Server-confirmed equipment state mismatch");
 need(s.moveItem({0,target,0,source}),"Restore failed; test item remains at printed destination");auto restored=s.state();need(restored.inventory->items.at(source).wire==original.wire&&!restored.inventory->items.count(target),"Restored inventory mismatch");if(equipped==6)need(restored.bootsLevel==(initiallyEquipped?int(original.level):-1),"Boot movement state stale after equipment move");s.disconnect();std::this_thread::sleep_for(std::chrono::seconds(2));
 Session again(profile,0);need(again.login(account,password)==1,"Relog failed");again.select("MuOptions");ready(again);auto after=again.state().inventory;need(after->items.at(source).wire==original.wire&&!after->items.count(target),"Equipment restoration did not persist");again.disconnect();std::cout<<"PASS: server equipment move, reverse move, original options retained and restoration persisted after relog.\n";return 0;
 }catch(const std::exception& e){std::cerr<<"Equipment test: "<<e.what()<<"\n";return 1;}}
