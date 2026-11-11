#include "assets.hpp"
#include "asset_images.hpp"
#import <Foundation/Foundation.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
namespace fs = std::filesystem;
std::string json(const std::string& s) {
 std::string out="\""; const char* hex="0123456789abcdef";
 for(unsigned char c:s) { if(c=='"'||c=='\\'){out+='\\';out+=c;} else if(c<32){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}else out+=c; } return out+'"';
}
int main(int argc,char** argv) { @autoreleasepool {
 if(argc!=2){std::cerr<<"Usage: asset-audit DATA > report.jsonl\n";return 2;}
 try {
 const auto root=fs::canonical(argv[1]);std::vector<fs::path> files;
 for(const auto& e:fs::recursive_directory_iterator(root))if(!e.is_symlink()&&e.is_regular_file())files.push_back(e.path());
 std::sort(files.begin(),files.end()); unsigned count=0;
 for(const auto& path:files) { @autoreleasepool {
 auto ext=path.extension().string();std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
 if(ext!=".bmd"&&ext!=".ozj"&&ext!=".ozt")continue;
 auto rel=fs::relative(path,root).generic_string();std::string kind=ext==".bmd"?"non_model_bmd":"texture";
 std::cout<<"{\"path\":"<<json(rel);
 try {
 if(ext==".bmd") {
 std::ifstream in(path,std::ios::binary);char sig[3]{};in.read(sig,3);
 if(std::string(sig,3)=="BMD") {
 kind="model";auto model=mu::assets::load_model(path,{});
 // An empty retained-action list validates layout without allocating every clip.
 // Skeleton traversal still checks parent cycles; animation floats remain unaudited.
 for(const auto& mesh:model.meshes)mu::assets::posed_vertices(mesh,model.bones);
 std::cout<<",\"actions\":"<<model.actionCount<<",\"bones\":"<<model.bones.size()<<",\"dependencies\":[";
 bool first=true;for(const auto& mesh:model.meshes){if(!first)std::cout<<',';first=false;
 std::cout<<"{\"reference\":"<<json(mesh.texture);
 try {auto resolved=mu::viewer::resolve_model_texture(path.parent_path(),mesh.texture);
 if(fs::is_symlink(resolved)||!fs::is_regular_file(resolved))throw std::runtime_error("Texture is not a regular non-symlink file");
 std::cout<<",\"resolved\":"<<json(fs::relative(resolved,root).generic_string())<<",\"status\":\"resolved\"";
 }catch(const std::exception& e){std::cout<<",\"status\":\"unresolved\",\"error\":"<<json(e.what());}std::cout<<'}';}
 std::cout<<']';
 }
 }else {auto image=mu::viewer::decode_texture(path);std::cout<<",\"width\":"<<image.width<<",\"height\":"<<image.height;}
 std::cout<<",\"kind\":"<<json(kind)<<",\"status\":"<<json(kind=="non_model_bmd"?"not_audited":"parsed")<<"}\n";
 }catch(const std::exception& e){std::cout<<",\"kind\":"<<json(kind)<<",\"status\":\"parser_error\",\"error\":"<<json(e.what())<<"}\n";}
 if(++count%500==0)std::cerr<<"Audited "<<count<<" candidates\n";
 }}return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
 }}
