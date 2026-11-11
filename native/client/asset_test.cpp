#include "assets.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <unistd.h>
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}need(rejected,"Malformed asset was accepted");}
int main(int argc,char** argv){try{
 need(argc==2,"Usage: MuClient-assets CLIENT_ROOT");std::filesystem::path data=std::filesystem::path(argv[1])/"Data";
 auto t=mu::assets::load_terrain(data,0);need(t.heights.size()==65536&&t.layer1[0]==5&&t.attributes.size()==65536,"Real Lorencia contract failed");
 need(std::abs(t.height(132,99)-1.65f)<1e-6&&std::abs(t.height(130,99)-.225f)<1e-6,"Bridge height regressed to standard BMP offset instead of legacy source offset");need(std::abs(t.heightAt(132.5,99.5)-1.65f)<1e-6,"Original bilinear bridge height failed");
 auto mini=mu::assets::decode_tga(mu::assets::read_file(data/"World1/mini_map.OZT"));need(mini.width==1024&&mini.height==1024,"Real minimap dimensions mismatch");
 auto image=mu::assets::jpeg_payload(mu::assets::read_file(data/"World1/TileGrass01.OZJ"));need(image[0]==255&&image[1]==216,"Real OZJ payload mismatch");
 rejects([]{mu::assets::decode_tga({0,0,0});});rejects([]{mu::assets::jpeg_payload({255,216});});
 auto damaged=mu::assets::read_file(data/"World1/mini_map.OZT");damaged.resize(30);rejects([&]{mu::assets::decode_tga(damaged);});
 auto skeleton=mu::assets::load_model(data/"Player/Player.bmd");unsigned triangles=0,parts=0;
 for(const char* name:{"HelmClass01.bmd","ArmorClass01.bmd","PantClass01.bmd","GloveClass01.bmd","BootClass01.bmd"}){
  auto model=mu::assets::load_model(data/"Player"/name);need(model.bones.size()<=skeleton.bones.size(),"Player skeleton is too short");
  for(auto& mesh:model.meshes)for(auto& vertex:mesh.vertices){auto i=unsigned(vertex.bone);need(model.bones[i].name==skeleton.bones[i].name&&model.bones[i].parent==skeleton.bones[i].parent,"Used bone/player skeleton mapping mismatch");}

  for(auto& mesh:model.meshes){auto posed=mu::assets::posed_vertices(mesh,skeleton.bones);float minZ=1e8,maxZ=-1e8;for(auto v:posed){need(std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z),"Nonfinite posed vertex");minZ=std::min(minZ,v.z);maxZ=std::max(maxZ,v.z);}std::cout<<name<<" z="<<minZ<<".."<<maxZ<<"\n";triangles+=mesh.faces.size();}parts++;
 }
 mu::assets::Mesh mesh;mesh.vertices.push_back({0,{1,2,3}});std::vector<mu::assets::Bone> bones(2);bones[0].parent=1;bones[1].parent=0;rejects([&]{mu::assets::posed_vertices(mesh,bones);});
 bones.resize(1);bones[0].parent=-1;bones[0].position={2,3,4};auto posed=mu::assets::posed_vertices(mesh,bones);need(posed[0].x==3&&posed[0].y==5&&posed[0].z==7,"Bone translation failed");
 std::cout<<"PASS: real Lorencia 256x256, two tile layers, height/attributes, minimap 1024x1024, OZJ, five player BMD parts, original skeleton; triangles="<<triangles<<"; malformed texture and cyclic skeleton rejected.\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<"Assets: "<<e.what()<<"\n";return 1;}}
