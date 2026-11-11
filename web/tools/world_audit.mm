#include "assets.hpp"
#include "localization.hpp"
#include "item_catalog.hpp"
#include "skill_catalog.hpp"
#import <Foundation/Foundation.h>
#include <filesystem>
#include <map>
#include <iostream>
namespace fs=std::filesystem;
NSString* str(const std::string& s){return [[NSString alloc] initWithBytes:s.data() length:s.size() encoding:NSUTF8StringEncoding] ?: @"<non-UTF8>";}
int main(int argc,char** argv){@autoreleasepool{
 if(argc!=2)return 2;
 try{
 auto root=fs::canonical(argv[1]);NSMutableArray* worlds=[NSMutableArray array];NSMutableArray* tables=[NSMutableArray array];
 for(auto& entry:fs::directory_iterator(root)){
 if(entry.is_symlink()||!entry.is_directory())continue;auto name=entry.path().filename().string();
 if(name.rfind("World",0)!=0||name.size()==5||name.find_first_not_of("0123456789",5)!=std::string::npos)continue;
 unsigned world=std::stoul(name.substr(5));if(!world)continue;unsigned map=world-1;
 NSMutableDictionary* row=[@{@"world":@(world),@"server_map":@(map)} mutableCopy];[worlds addObject:row];
 if(map!=0&&map!=3&&map!=51&&map!=73&&map!=74){row[@"status"]=@"unsupported_by_native_parser";continue;}
 try{auto t=mu::assets::load_terrain(root,map);row[@"terrain"]=@{@"status":@"parsed",@"heights":@(t.heights.size()),@"attributes":@(t.attributes.size())};}
 catch(const std::exception& e){row[@"terrain"]=@{@"status":@"parser_error",@"error":str(e.what())};}
 try{auto objects=mu::assets::load_world_objects(root,map);NSMutableArray* dependencies=[NSMutableArray array];std::map<unsigned,unsigned> types;
 for(auto& o:objects.objects)types[o.type]++;
 for(auto [type,count]:types){bool marker=(map==51&&(type==0||type==54||(type>=58&&type<=62)))||(map==3&&type==38)||(map==0&&type>=130&&type<=133);
 auto file=map==0?mu::assets::lorencia_model_name(type):"Object"+std::string(type+1<10?"0":"")+std::to_string(type+1)+".bmd";
 auto path=fs::path("Object"+std::to_string(world))/file;
 auto status=marker?"native_marker_skipped":file.empty()?"unmapped_type":fs::is_regular_file(root/path)&&!fs::is_symlink(root/path)?"present":"missing";
 [dependencies addObject:@{@"type":@(type),@"placements":@(count),@"model":str(path.generic_string()),@"status":str(status)}];}
 row[@"objects"]=@{@"status":@"parsed",@"declared":@(objects.declaredCount),@"records":@(objects.recordCount),@"active":@(objects.objects.size()),@"dependencies":dependencies};}
 catch(const std::exception& e){row[@"objects"]=@{@"status":@"parser_error",@"error":str(e.what())};}
 row[@"status"]=@"audited_supported_world";
 }
 for(auto lang:{"eng","por","spn"})for(auto stem:{"Text","Item","Skill"}){
 auto path=mu::ui::local_data_file(root,lang,stem);NSMutableDictionary* row=[@{@"path":str(fs::relative(path,root).generic_string())} mutableCopy];[tables addObject:row];
 try{size_t count=0;if(std::string(stem)=="Text")count=mu::ui::load_texts(root,lang).size();else if(std::string(stem)=="Item")count=mu::ui::load_item_catalog(root,lang)->size();else count=mu::ui::load_skill_catalog(root,lang).size();row[@"status"]=@"parsed";row[@"entries"]=@(count);}
 catch(const std::exception& e){row[@"status"]=@"parser_error";row[@"error"]=str(e.what());}
 }
 auto report=@{@"worlds":worlds,@"tables":tables};NSError* error=nil;NSData* json=[NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted|NSJSONWritingSortedKeys error:&error];if(!json)return 1;std::cout.write((const char*)json.bytes,json.length);std::cout<<'\n';return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}}
