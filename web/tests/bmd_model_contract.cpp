#include "mu/bmd_model.hpp"
#include "mu/bmd_geometry.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_BMD_NATIVE_REFERENCE
#include "assets.hpp"
#include <fstream>
#endif
using namespace mu::core::bmd;
void word(Bytes& b,int n){b.push_back(n&255);b.push_back((n>>8)&255);}
void number(Bytes& b,float f){std::uint32_t n;std::memcpy(&n,&f,4);for(int i=0;i<4;i++)b.push_back((n>>(8*i))&255);}
void vector(Bytes& b,float x=0){number(b,x);number(b,0);number(b,0);}
void name(Bytes& b,const char* text){std::size_t start=b.size();for(;*text;text++)b.push_back(*text);b.resize(start+32,0);}
Bytes fixture(bool lock=false){
 Bytes b{'B','M','D',10};name(b,"fixture");word(b,1);word(b,1);word(b,1);
 word(b,3);word(b,1);word(b,3);word(b,1);word(b,0);
 for(int i=0;i<3;i++){word(b,0);word(b,0);vector(b,float(i));}
 word(b,0);word(b,0);vector(b,1);word(b,0);word(b,0);
 for(int i=0;i<3;i++){number(b,float(i));number(b,0);}
 auto start=b.size();b.push_back(3);b.push_back(0);for(int i:{0,1,2,0})word(b,i);for(int i=0;i<4;i++)word(b,0);for(int i:{0,1,2,0})word(b,i);b.resize(start+64,0);
 name(b,"fixture.tga");word(b,1);b.push_back(lock);if(lock)vector(b);
 b.push_back(0);name(b,"root");word(b,-1);vector(b,2);vector(b);return b;
}
Bytes encrypted(const Bytes& b){
 constexpr unsigned key[]={0xD1,0x73,0x52,0xF6,0xD2,0x9A,0xCB,0x27,0x3E,0xAF,0x59,0x31,0x37,0xB3,0xE7,0xA2};
 Bytes out{'B','M','D',12};unsigned size=unsigned(b.size()-4);for(int i=0;i<4;i++)out.push_back((size>>(8*i))&255);unsigned prev=0x5e;
 for(unsigned i=4;i<b.size();i++){auto byte=std::uint8_t(((b[i]+prev)&255)^key[(i-4)%16]);out.push_back(byte);prev=(byte+0x3d)&255;}return out;
}
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}assert(failed);}
#ifdef MU_BMD_NATIVE_REFERENCE
void compare(const Model& a,const mu::assets::Model& b){
 assert(a.actionCount==b.actionCount&&a.meshes.size()==b.meshes.size()&&a.bones.size()==b.bones.size());
 auto geometry=initial_geometry(a);
 for(unsigned i=0;i<a.meshes.size();i++){
  const auto& x=a.meshes[i];const auto& y=b.meshes[i];assert(x.texture==y.texture&&x.textureIndex==y.textureIndex&&x.vertices.size()==y.vertices.size()&&x.normals.size()==y.normals.size()&&x.uv==y.uv&&x.faces.size()==y.faces.size());
  for(unsigned v=0;v<x.vertices.size();v++){assert(x.vertices[v].bone==y.vertices[v].bone);assert(x.vertices[v].position.x==y.vertices[v].position.x&&x.vertices[v].position.y==y.vertices[v].position.y&&x.vertices[v].position.z==y.vertices[v].position.z);}
  auto posed=mu::assets::posed_vertices(y,b.bones);const auto& output=geometry.meshes[i];unsigned corner=0;
  for(const auto& face:y.faces)for(auto index:face.vertex){auto p=posed[index];assert(std::abs(output.positions[corner*3]-p.x)<0.0001f&&std::abs(output.positions[corner*3+1]-p.y)<0.0001f&&std::abs(output.positions[corner*3+2]-p.z)<0.0001f);corner++;}
  for(unsigned f=0;f<x.faces.size();f++)assert(x.faces[f].vertex==y.faces[f].vertex&&x.faces[f].normal==y.faces[f].normal&&x.faces[f].uv==y.faces[f].uv);
 }
 for(unsigned i=0;i<a.bones.size();i++)assert(a.bones[i].parent==b.bones[i].parent&&a.bones[i].name==b.bones[i].name&&a.bones[i].position.x==b.bones[i].position.x);
}
#endif
int main(int argc,char** argv){
 for(bool locked:{false,true}){
  auto bytes=fixture(locked);for(const auto& b:{bytes,encrypted(bytes)}){auto m=decode_model(b);
#ifdef MU_BMD_NATIVE_REFERENCE
   auto fixturePath=std::filesystem::path(argv[0]).parent_path()/"fixture.bmd";
   {std::ofstream stream(fixturePath,std::ios::binary);stream.write(reinterpret_cast<const char*>(b.data()),b.size());}
   compare(m,mu::assets::load_model(fixturePath));std::filesystem::remove(fixturePath);
#endif
   auto geometry=initial_geometry(m);assert(geometry.meshes[0].positions.size()==9&&geometry.meshes[0].positions[0]==2&&geometry.meshes[0].positions[6]==4&&geometry.meshes[0].normals[0]==1);
   assert(m.meshes.size()==1&&m.meshes[0].faces.size()==1&&m.meshes[0].vertices[2].position.x==2&&m.bones[0].position.x==2);}
  for(unsigned size=0;size<bytes.size();size++)rejects([&]{decode_model(Bytes(bytes.begin(),bytes.begin()+size));});
 }
 Model hierarchy;hierarchy.bones.resize(2);hierarchy.bones[0].rotation.z=1.57079632679f;hierarchy.bones[0].position={10,0,0};hierarchy.bones[1].parent=0;hierarchy.bones[1].position={1,0,0};Mesh mesh;mesh.vertices.push_back({1,{1,0,0}});mesh.normals.push_back({1,{2,0,0}});mesh.uv.push_back({0,0});mesh.faces.push_back({{0,0,0},{0,0,0},{0,0,0}});hierarchy.meshes.push_back(mesh);auto rotated=initial_geometry(hierarchy);assert(std::abs(rotated.meshes[0].positions[0]-10)<0.0001f&&std::abs(rotated.meshes[0].positions[1]-2)<0.0001f&&std::abs(rotated.meshes[0].normals[1]-1)<0.0001f);
 Model boundSource=hierarchy;boundSource.clips[1]={boundSource.bones};auto binding=bound_geometry(hierarchy,boundSource,1,0);assert(std::abs(binding.meshes[0].positions[1]-2)<0.0001f);rejects([&]{bound_geometry(hierarchy,boundSource,2,0);});rejects([&]{bound_geometry(hierarchy,boundSource,1,1);});auto wrong=hierarchy;wrong.bones[1].name="wrong";rejects([&]{bound_geometry(wrong,boundSource,1,0);});
 auto parent=fixture();parent[parent.size()-26]=0;parent[parent.size()-25]=0;rejects([&]{decode_model(parent);});
 auto b=fixture();b.push_back(0);rejects([&]{decode_model(b);});b=fixture();b[3]=0;rejects([&]{decode_model(b);});b=fixture();b[56]=0;b[57]=0;b[58]=0xc0;b[59]=0x7f;rejects([&]{decode_model(b);});
#ifdef MU_BMD_NATIVE_REFERENCE
 for(int i=1;i<argc;i++){
  std::ifstream input(argv[i],std::ios::binary);Bytes bytes((std::istreambuf_iterator<char>(input)),{});
  auto model=decode_model(bytes);compare(model,mu::assets::load_model(argv[i]));
  if(argc==5&&i==3){auto player=decode_model(mu::assets::read_file(argv[4]),{1});auto bound=bound_geometry(model,player,1,0);auto reference=mu::assets::load_model(argv[4],{1});auto armor=mu::assets::load_model(argv[i]);for(unsigned m=0;m<armor.meshes.size();m++){auto points=mu::assets::posed_vertices(armor.meshes[m],reference.clips.at(1).at(0));unsigned c=0;for(const auto& face:armor.meshes[m].faces)for(auto vi:face.vertex){auto p=points[vi];assert(std::abs(bound.meshes[m].positions[c*3]-p.x)<0.0001f&&std::abs(bound.meshes[m].positions[c*3+1]-p.y)<0.0001f&&std::abs(bound.meshes[m].positions[c*3+2]-p.z)<0.0001f);c++;}}std::cout<<"Player-bound pose matches native\n";}
  std::cout<<"asset passed: "<<i<<" meshes="<<model.meshes.size()<<" bones="<<model.bones.size()<<" actions="<<model.actionCount<<'\n';
 }
#else
 (void)argc;(void)argv;
#endif
 std::cout<<"BMD model contract passed\n";
}
