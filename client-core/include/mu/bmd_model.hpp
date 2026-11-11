#include "bmd_empty_mesh.hpp"
#pragma once
#include <array>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <algorithm>
namespace mu::core::bmd {
using Bytes=std::vector<std::uint8_t>;
struct Vec3 {float x,y,z;};
struct Vertex {int bone;Vec3 position;};
struct Face {std::array<unsigned,3> vertex,uv;std::array<unsigned,3> normal{};};
struct Mesh {int textureIndex=0;std::string texture;std::vector<Vertex> vertices,normals;std::vector<std::array<float,2>> uv;std::vector<Face> faces;};
struct Bone {int parent=-1;bool dummy=true;std::string name;Vec3 position{},rotation{};};
struct Model {unsigned actionCount=0;std::map<unsigned,bool> lockedPositions;std::vector<Mesh> meshes;std::vector<Bone> bones;std::vector<std::vector<Bone>> idlePoses;std::map<unsigned,std::vector<std::vector<Bone>>> clips;};
namespace {
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
unsigned u16(const Bytes& b,std::size_t n){need(n+2<=b.size(),"Truncated asset integer");return b[n]|(unsigned(b[n+1])<<8);}
unsigned u32(const Bytes& b,std::size_t n){return u16(b,n)|(u16(b,n+2)<<16);}
class Reader {
 const Bytes& bytes;std::size_t pos=0;
public:
 Reader(const Bytes& b):bytes(b){}
 void skip(std::size_t n){need(n<=bytes.size()-pos,"Truncated model data");pos+=n;}
 unsigned byte(){need(pos<bytes.size(),"Truncated model data");return bytes[pos++];}
 int small(){auto n=u16(bytes,pos);pos+=2;return static_cast<std::int16_t>(n);}
 float number(){need(pos+4<=bytes.size(),"Truncated model float");auto bits=u32(bytes,pos);float n;std::memcpy(&n,&bits,4);pos+=4;need(std::isfinite(n)&&std::abs(n)<1e7,"Invalid model float");return n;}
 Vec3 vector(){return {number(),number(),number()};}
 std::string name(){need(pos+32<=bytes.size(),"Truncated model string");auto p=reinterpret_cast<const char*>(bytes.data()+pos);std::string s(p,std::find(p,p+32,'\0'));pos+=32;return s;}
 std::size_t offset()const{return pos;}
};
}
inline Bytes decrypt_file(const Bytes& input){
 constexpr std::uint8_t key[16]={0xD1,0x73,0x52,0xF6,0xD2,0x9A,0xCB,0x27,0x3E,0xAF,0x59,0x31,0x37,0xB3,0xE7,0xA2};
 Bytes out(input.size());unsigned previous=0x5E;
 for(std::size_t i=0;i<input.size();i++){out[i]=std::uint8_t((input[i]^key[i%16])-previous);previous=(input[i]+0x3D)&255;}return out;
}
inline Model decode_model(const Bytes& b,const std::vector<unsigned>& retainedActions={0}){
 need(b.size()<=16*1024*1024,"Model byte limit exceeded");need(b.size()>=42&&b[0]=='B'&&b[1]=='M'&&b[2]=='D',"Invalid BMD model signature");
 Bytes payload;
 if(b[3]==12){auto size=u32(b,4);need(size==b.size()-8,"BMD encrypted size mismatch");payload=decrypt_file(Bytes(b.begin()+8,b.end()));}
 else{need(b[3]==10,"Unsupported BMD model version");payload=Bytes(b.begin()+4,b.end());}
 Reader r(payload);r.name();int meshes=r.small(),bones=r.small(),actions=r.small();need(meshes>=0&&meshes<=128&&bones>=0&&bones<=256&&actions>=0&&actions<=512,"Invalid BMD model counts");Model model;model.actionCount=actions;
 std::size_t records=0;
 for(int m=0;m<meshes;m++){
  int vertices=r.small(),normals=r.small(),uvs=r.small(),faces=r.small();int textureIndex=r.small();need(vertices>=0&&vertices<=30000&&normals>=0&&uvs>=0&&faces>=0,"Invalid BMD mesh counts");records+=std::size_t(vertices)+normals+uvs+std::size_t(faces)*2;need(records<=262144,"Model geometry allocation exceeds bound");Mesh mesh;mesh.textureIndex=textureIndex;
  for(int v=0;v<vertices;v++){int node=r.small();r.skip(2);auto pos=r.vector();mesh.vertices.push_back({node,pos});}
  for(int n=0;n<normals;n++){int node=r.small();r.skip(2);auto normal=r.vector();r.skip(4);mesh.normals.push_back({node,normal});}for(int t=0;t<uvs;t++)mesh.uv.push_back({r.number(),r.number()});
  for(int f=0;f<faces;f++){
   auto start=r.offset();unsigned polygon=r.byte();r.skip(1);std::array<int,4> vi{},ti{},ni{};
   for(auto& i:vi)i=r.small();for(auto& i:ni)i=r.small();for(auto& i:ti)i=r.small();r.skip(64-(r.offset()-start));
   need(polygon==3||polygon==4,"Unsupported BMD polygon");
   for(unsigned i=0;i<polygon;i++)need(vi[i]>=0&&vi[i]<vertices&&ti[i]>=0&&ti[i]<uvs&&ni[i]>=0&&ni[i]<normals,"BMD face index out of bounds");
   for(unsigned i=1;i+1<polygon;i++)mesh.faces.push_back({{unsigned(vi[0]),unsigned(vi[i]),unsigned(vi[i+1])},{unsigned(ti[0]),unsigned(ti[i]),unsigned(ti[i+1])},{unsigned(ni[0]),unsigned(ni[i]),unsigned(ni[i+1])}});
  }
  mu::bmd_validation::discard_empty_sentinel_faces(mesh);
  for(const auto& v:mesh.vertices)need(v.bone>=-1&&v.bone<bones,"BMD vertex bone index out of bounds");
  for(const auto& n:mesh.normals)need(n.bone>=-1&&n.bone<bones,"BMD normal bone index out of bounds");
  mesh.texture=r.name();need(mesh.texture.find('/')==std::string::npos&&mesh.texture.find('\\')==std::string::npos&&mesh.texture.find("..") == std::string::npos,"Unsafe model texture name");model.meshes.push_back(std::move(mesh));
 }
 std::vector<int> keys;std::size_t total=0;
 for(int a=0;a<actions;a++){int count=r.small();unsigned lock=r.byte();need(count>0&&count<=4096&&lock<=1,"Invalid BMD action");total+=count;need(total<=100000,"BMD animation size too large");keys.push_back(count);model.lockedPositions[a]=lock!=0;if(lock)for(int k=0;k<count;k++)r.vector();}
 std::size_t retainedBones=0;
 for(auto action:retainedActions){if(actions==0&&action==0)continue;need(action<unsigned(actions)&&keys[action]<=256,"Requested model action unavailable or too large");retainedBones+=std::size_t(keys[action])*bones;need(retainedBones<=32768,"Retained animation allocation exceeds bound");model.clips[action].resize(keys[action],std::vector<Bone>(bones));}
 for(int n=0;n<bones;n++){
  Bone bone;auto dummy=r.byte();need(dummy<=1,"Invalid BMD bone flag");bone.dummy=dummy!=0;
  if(!bone.dummy){bone.name=r.name();bone.parent=r.small();need(bone.parent>=-1&&bone.parent<bones&&bone.parent!=n,"Invalid BMD bone parent");
   for(auto& clip:model.clips)for(auto& pose:clip.second)pose[n]=bone;
   for(int a=0;a<actions;a++){
    if(model.clips.count(a)){auto& poses=model.clips.at(a);for(auto& pose:poses)pose[n].position=r.vector();for(auto& pose:poses)pose[n].rotation=r.vector();if(a==0){bone.position=poses[0][n].position;bone.rotation=poses[0][n].rotation;}}
    else for(int k=0;k<keys[a]*2;k++)r.vector();
   }
  }model.bones.push_back(bone);
 }
 // Check every parent chain, including cycles longer than a self-parent.
 for(std::size_t n=0;n<model.bones.size();n++){std::vector<bool> seen(model.bones.size());int p=int(n);while(p>=0){need(!seen[unsigned(p)],"Cyclic model bone hierarchy");seen[unsigned(p)]=true;p=model.bones[unsigned(p)].parent;}}
 need(r.offset()==payload.size(),"Unexpected trailing BMD data");if(model.clips.count(0))model.idlePoses=model.clips.at(0);return model;
}
}
