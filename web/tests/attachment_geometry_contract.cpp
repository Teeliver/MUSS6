#include "mu/attachment_geometry.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_ATTACHMENT_REFERENCE
#include "assets.hpp"
#include "equipment_grip.hpp"
#include <fstream>
#include <functional>
#endif
using namespace mu::core::bmd;
template<class F>void rejects_attachment(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}assert(failed);}
#ifdef MU_ATTACHMENT_REFERENCE
void reference(const Model& item,const Model& player,const mu::assets::Model& nativeItem,const mu::assets::Model& nativePlayer,unsigned action,unsigned slot,bool safe,unsigned itemIndex=0){
 auto geometry=attachment_geometry(item,player,action,0,slot,itemIndex,safe);const auto& bones=nativePlayer.clips.at(action)[0];
 std::function<simd_float4x4(unsigned)> world=[&](unsigned i){auto b=bones.at(i);auto q=simd_mul(simd_quaternion(b.rotation.z,simd_make_float3(0,0,1)),simd_mul(simd_quaternion(b.rotation.y,simd_make_float3(0,1,0)),simd_quaternion(b.rotation.x,simd_make_float3(1,0,0))));auto m=simd_matrix4x4(q);m.columns[3]=simd_make_float4(b.position.x,b.position.y,b.position.z,1);return b.parent<0?m:simd_mul(world(unsigned(b.parent)),m);};
 auto matrix=simd_mul(world(mu::viewer::equipment_attachment_bone(slot,itemIndex,safe)),mu::viewer::equipment_attachment_transform(slot,itemIndex,safe));
 for(unsigned m=0;m<nativeItem.meshes.size();m++){const auto& mesh=nativeItem.meshes[m];auto vertices=mu::assets::posed_vertices(mesh,nativeItem.bones);unsigned corner=0;
 for(const auto& face:mesh.faces)for(auto index:face.vertex){auto p=vertices[index];auto out=simd_mul(matrix,simd_make_float4(p.x,p.y,p.z,1));for(unsigned r=0;r<3;r++)assert(std::abs(out[r]-geometry.meshes[m].positions[corner*3+r])<0.002f);corner++;}}
}
#endif
int main(int argc,char** argv){
 Model item,player;item.bones.resize(1);Mesh mesh;mesh.vertices.push_back({0,{1,0,0}});mesh.normals.push_back({0,{1,0,0}});mesh.uv.push_back({0,0});mesh.faces.push_back({{0,0,0},{0,0,0},{0,0,0}});item.meshes.push_back(mesh);
 player.bones.resize(48);for(auto& b:player.bones)b.parent=-1;player.bones[33].position={10,0,0};player.bones[33].rotation.z=1.57079632679f;player.clips[1]={player.bones};
 auto attached=attachment_geometry(item,player,1,0,0,0,false);assert(std::abs(attached.meshes[0].positions[0]-10)<0.0001f);assert(std::abs(attached.meshes[0].positions[1]-1)<0.0001f);assert(std::abs(attached.meshes[0].normals[0])<0.0001f&&std::abs(attached.meshes[0].normals[1]-1)<0.0001f);
 player.clips[1][0][47].rotation.z=1.57079632679f;
 auto wing=item;wing.clips[0]={wing.bones,wing.bones};wing.clips[0][1][0].position.x=10;wing.lockedPositions[0]=false;
 auto wingA=attachment_geometry(wing,player,1,0,7,6145,true,0),wingB=attachment_geometry(wing,player,1,0,7,6145,true,.16);
 assert(std::abs(wingA.meshes[0].positions[1]-1)<.001);
 assert(std::abs(wingB.meshes[0].positions[1]-wingA.meshes[0].positions[1]-10)<.001);
 auto texturedWing=wing;texturedWing.meshes[0].texture="angel_wing.tga";texturedWing.meshes[0].uv[0][1]=.2f;
 auto textureGeometry=attachment_geometry(texturedWing,player,1,0,7,6145,true,0);
 assert(std::abs(textureGeometry.meshes[0].uv[1]-.8f)<.001);
 auto satanMask=attachment_geometry(texturedWing,player,1,0,8,6657,false,0);
 assert(std::abs(satanMask.meshes[0].uv[1]-.8f)<.001);
 auto texturedSword=attachment_geometry(texturedWing,player,1,0,0,0,false,0);
 assert(std::abs(texturedSword.meshes[0].uv[1]-.2f)<.001);
 auto upright=wing;upright.meshes[0].vertices[0].position={0,3,2};
 auto uprightGeometry=attachment_geometry(upright,player,1,0,7,6145,true,0);
 assert(std::abs(uprightGeometry.meshes[0].positions[0]+3)<.001);
 assert(std::abs(uprightGeometry.meshes[0].positions[2]-17)<.001);
 auto slowFlight=attachment_geometry(wing,player,1,0,7,12*512+36,false,.08,true);
 assert(std::abs(slowFlight.meshes[0].positions[1]-wingB.meshes[0].positions[1])<.001);
 auto feather=attachment_geometry(wing,player,1,0,7,12*512+39,false,1./3.75);
 assert(std::abs(feather.meshes[0].positions[1]-wingB.meshes[0].positions[1])<.001);
 auto wingFast=attachment_geometry(wing,player,1,0,7,6145,false,.04,true);assert(std::abs(wingFast.meshes[0].positions[0]-wingB.meshes[0].positions[0])<.001);
 rejects_attachment([&]{attachment_geometry(item,player,2,0,0,0,false);});rejects_attachment([&]{attachment_geometry(item,player,1,1,0,0,false);});rejects_attachment([&]{attachment_geometry(item,player,1,0,2,0,false);});
 auto satan=attachment_geometry(item,player,1,0,8,6657,false);assert(satan.meshes.size()==1);assert(mu::core::equipment_grip(8,6657,false).bone==34);
 auto rfSatan=attachment_geometry(item,player,1,0,8,6657,false,0,false,true);
 assert(std::abs(rfSatan.meshes[0].positions[1]-satan.meshes[0].positions[1]+5)<.001);
 assert(std::abs(rfSatan.meshes[0].positions[2]-satan.meshes[0].positions[2]-20)<.001);
 auto shortRig=player;shortRig.clips[1][0].resize(1);rejects_attachment([&]{attachment_geometry(item,shortRig,1,0,0,0,false);});player.bones[33].parent=33;rejects_attachment([&]{bone_world_matrix(player.bones,33);});
#ifdef MU_ATTACHMENT_REFERENCE
 if(argc>1){auto data=std::filesystem::path(argv[1]);auto read=[](auto path){std::ifstream f(path,std::ios::binary);assert(f.good());return Bytes(std::istreambuf_iterator<char>(f),{});};
 auto itemPath=data/"Item/Sword01.bmd",playerPath=data/"Player/Player.bmd";auto sword=decode_model(read(itemPath)),rig=decode_model(read(playerPath),{1,2,3});auto ni=mu::assets::load_model(itemPath),np=mu::assets::load_model(playerPath,{1,2,3});
 for(unsigned action:{1u,2u,3u})for(unsigned slot:{0u,1u})for(bool safe:{false,true})reference(sword,rig,ni,np,action,slot,safe);
 for(const auto& fixture:std::array<std::pair<const char*,unsigned>,4>{{{"Bow01.bmd",2048},{"Arrows02.bmd",2063},{"Staff01.bmd",2560},{"Shield01.bmd",3072}}}){
  auto path=data/"Item"/fixture.first;auto portable=decode_model(read(path));auto original=mu::assets::load_model(path);auto rig2=decode_model(read(playerPath),{4,8,17,21,26,30,39,50});auto nativeRig=mu::assets::load_model(playerPath,{4,8,17,21,26,30,39,50});bool bow=fixture.second/512==4;unsigned hand=(fixture.second==2048||fixture.second==3072)?1:0;
  for(unsigned action:bow?std::vector<unsigned>{8,21,30,50}:std::vector<unsigned>{4,17,26,39})for(bool zone:{false,true})reference(portable,rig2,original,nativeRig,action,hand,zone,fixture.second);
 }
 std::cout<<"Bow/arrows/staff/shield native geometry reference: 32 poses passed\n";
 std::cout<<"Actual Sword01/Player attachment native reference: 12 poses passed\n";}
#else
 (void)argc;(void)argv;
#endif
 std::cout<<"attachment geometry contract passed\n";
}
