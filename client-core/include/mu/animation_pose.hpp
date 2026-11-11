#pragma once
#include "bmd_model.hpp"
#include <array>
#include <algorithm>
namespace mu::core::bmd {
using Quaternion=std::array<double,4>;
inline Quaternion pose_quaternion(Vec3 r){double x=r.x*.5,y=r.y*.5,z=r.z*.5,cx=std::cos(x),sx=std::sin(x),cy=std::cos(y),sy=std::sin(y),cz=std::cos(z),sz=std::sin(z);return {sx*cy*cz-cx*sy*sz,cx*sy*cz+sx*cy*sz,cx*cy*sz-sx*sy*cz,cx*cy*cz+sx*sy*sz};}
inline Vec3 pose_rotation(Quaternion q){double n=0;for(auto v:q)n+=v*v;need(n>0&&std::isfinite(n),"Invalid pose quaternion");for(auto& v:q)v/=std::sqrt(n);auto [x,y,z,w]=q;return {float(std::atan2(2*(w*x+y*z),1-2*(x*x+y*y))),float(std::asin(std::clamp(2*(w*y-z*x),-1.,1.))),float(std::atan2(2*(w*z+x*y),1-2*(y*y+z*z)))};}
inline std::vector<Bone> blend_pose(const std::vector<Bone>& fromPose,const std::vector<Bone>& toPose,double t){
 need(fromPose.size()==toPose.size()&&std::isfinite(t)&&t>=0&&t<=1,"Invalid animation blend");auto result=fromPose;
 for(unsigned i=0;i<result.size();i++){const auto& from=fromPose[i];const auto& to=toPose[i];auto qa=pose_quaternion(from.rotation),qb=pose_quaternion(to.rotation);double dot=0;for(unsigned j=0;j<4;j++)dot+=qa[j]*qb[j];if(dot<0){for(auto& v:qb)v=-v;dot=-dot;}double wa=1-t,wb=t;if(dot<.9995){double angle=std::acos(std::clamp(dot,-1.,1.)),s=std::sin(angle);wa=std::sin((1-t)*angle)/s;wb=std::sin(t*angle)/s;}Quaternion q{};for(unsigned j=0;j<4;j++)q[j]=qa[j]*wa+qb[j]*wb;
 result[i].position={float(from.position.x*(1-t)+to.position.x*t),float(from.position.y*(1-t)+to.position.y*t),float(from.position.z*(1-t)+to.position.z*t)};result[i].rotation=pose_rotation(q);
 }return result;
}
inline std::vector<Bone> sample_pose(const Model& player,unsigned action,double seconds,double keysPerSecond=6.25){
 need(std::isfinite(seconds)&&seconds>=0&&std::isfinite(keysPerSecond)&&keysPerSecond>0,"Invalid animation clock");auto clip=player.clips.find(action);need(clip!=player.clips.end()&&!clip->second.empty(),"Animation clip unavailable");
 unsigned period=unsigned(clip->second.size());auto lock=player.lockedPositions.find(action);if(lock!=player.lockedPositions.end()&&lock->second&&period>1)period--;
 double frame=std::fmod(seconds*keysPerSecond,double(period));need(std::isfinite(frame),"Animation clock overflow");unsigned a=unsigned(frame),b=(a+1)%period;double t=frame-a;
 auto result=clip->second[a];need(result.size()==clip->second[b].size(),"Animation bone count mismatch");
 return blend_pose(clip->second[a],clip->second[b],t);
}
}
