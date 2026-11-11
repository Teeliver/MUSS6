#include "mu/animation_pose.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_ANIMATION_REFERENCE
#include "assets.hpp"
#include <simd/simd.h>
#include <fstream>
#endif
using namespace mu::core::bmd;
int main(int argc,char** argv){
 Model model;model.bones.resize(1);auto a=model.bones,b=model.bones;a[0].rotation.z=3.f;b[0].rotation.z=-3.f;b[0].position.x=10;model.clips[4]={a,b};
 auto mid=sample_pose(model,4,.08);assert(std::abs(mid[0].position.x-5)<.0001f);assert(std::abs(std::abs(mid[0].rotation.z)-3.14159265f)<.0001f);
 auto loop=sample_pose(model,4,.32);assert(std::abs(loop[0].position.x)<.0001f);
 model.lockedPositions[4]=true;assert(sample_pose(model,4,100)[0].position.x==0);
 for(double bad:{-1.,std::numeric_limits<double>::infinity()}){bool fail=false;try{sample_pose(model,4,bad);}catch(const std::exception&){fail=true;}assert(fail);}
#ifdef MU_ANIMATION_REFERENCE
 if(argc>1){auto path=std::filesystem::path(argv[1])/"Player/Player.bmd";std::ifstream stream(path,std::ios::binary);assert(stream.good());Bytes bytes(std::istreambuf_iterator<char>(stream),{});auto player=decode_model(bytes,{1,2,3,4,146,147});auto native=mu::assets::load_model(path,{1,2,3,4,146,147});unsigned checked=0;
 for(unsigned action:{1u,2u,3u,4u,146u,147u})for(unsigned tick=0;tick<240;tick++){
 double seconds=tick/120.;auto pose=sample_pose(player,action,seconds);auto& frames=native.clips.at(action);unsigned period=frames.size()-(native.lockedPositions.at(action)&&frames.size()>1?1:0);double frame=fmod(seconds*6.25,double(period));unsigned from=frame,to=(from+1)%period;float t=frame-from;
 auto quaternion=[](const mu::assets::Bone& bone){auto r=bone.rotation;return simd_mul(simd_quaternion(r.z,simd_make_float3(0,0,1)),simd_mul(simd_quaternion(r.y,simd_make_float3(0,1,0)),simd_quaternion(r.x,simd_make_float3(1,0,0))));};
 for(unsigned bone=0;bone<pose.size();bone++){auto q=simd_normalize(simd_slerp(quaternion(frames[from][bone]),quaternion(frames[to][bone]),t));auto actual=pose_quaternion(pose[bone].rotation);double dot=0;for(unsigned j=0;j<4;j++)dot+=actual[j]*q.vector[j];assert(std::abs(dot)>0.99999);auto p=simd_mix(simd_make_float3(frames[from][bone].position.x,frames[from][bone].position.y,frames[from][bone].position.z),simd_make_float3(frames[to][bone].position.x,frames[to][bone].position.y,frames[to][bone].position.z),t);assert(std::abs(pose[bone].position.x-p.x)<.0002f&&std::abs(pose[bone].position.y-p.y)<.0002f&&std::abs(pose[bone].position.z-p.z)<.0002f);checked++;}
 }std::cout<<"Native animation reference passed: "<<checked<<" bone samples at 120 Hz\n";}
#else
 (void)argc;(void)argv;
#endif
 std::cout<<"animation pose contract passed\n";
}
