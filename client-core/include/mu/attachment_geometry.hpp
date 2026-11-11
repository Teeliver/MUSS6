#pragma once
#include "bmd_geometry.hpp"
#include "animation_pose.hpp"
#include "equipment_grip.hpp"
namespace mu::core::bmd {
inline GripMatrix bone_world_matrix(const std::vector<Bone>& bones,unsigned bone){
 need(bone<bones.size(),"Attachment bone unavailable");std::vector<unsigned> chain;std::vector<bool> seen(bones.size());int index=int(bone);
 while(index>=0){need(unsigned(index)<bones.size()&&!seen[unsigned(index)],"Invalid attachment bone hierarchy");seen[unsigned(index)]=true;chain.push_back(unsigned(index));index=bones[unsigned(index)].parent;}
 auto world=grip_identity();
 for(auto it=chain.rbegin();it!=chain.rend();++it){const auto& b=bones[*it];const float x=b.rotation.x,y=b.rotation.y,z=b.rotation.z,cx=std::cos(x),sx=std::sin(x),cy=std::cos(y),sy=std::sin(y),cz=std::cos(z),sz=std::sin(z);
 GripMatrix local={cz*cy,sz*cy,-sy,0,cz*sy*sx-sz*cx,sz*sy*sx+cz*cx,cy*sx,0,cz*sy*cx+sz*sx,sz*sy*cx-cz*sx,cy*cx,0,b.position.x,b.position.y,b.position.z,1};world=grip_multiply(world,local);}
 for(float v:world)need(std::isfinite(v),"Invalid attachment pose");return world;
}
inline Geometry attachment_geometry(const Model& item,const Model& player,unsigned action,unsigned frame,unsigned slot,unsigned index,bool safe,double seconds=0,bool flying=false,bool rfSatan=false){
 const auto clip=player.clips.find(action);need(clip!=player.clips.end()&&frame<clip->second.size(),"Attachment Player pose unavailable");
 if(slot==7&&(index==13*512+30||index==12*512+49))return {};
 auto grip=equipment_grip(slot,index,safe);if(slot==8&&rfSatan)grip.transform=grip_euler({0,0,0},{20,-5,20});auto matrix=grip_multiply(bone_world_matrix(clip->second[frame],grip.bone),grip.transform);
 // RotationPosition in native BMD copies the owner bone into ParentMatrix even for Link=false.
 const bool animatedWing=((slot==7&&index!=12*512+40)||slot==8)&&item.clips.count(0);
 auto geometry=animatedWing?geometry_from_pose(item.meshes,sample_pose(item,0,seconds,slot==8?12.5:index==12*512+39?3.75:flying?(index==12*512+36?12.5:25.):6.25)):initial_geometry(item);
 if(slot==7&&(index==12*512+40||index==12*512+50)&&geometry.meshes.size()>1)geometry.meshes.resize(1);
 for(auto& mesh:geometry.meshes){
  // OZT is decoded to top-first RGBA; wing BMD UVs address its original bottom-first rows.
  if((slot==7||(slot==8&&index==13*512+1))&&mesh.texture.size()>=4&&mesh.texture.substr(mesh.texture.size()-4)==".tga")for(unsigned i=1;i<mesh.uv.size();i+=2)mesh.uv[i]=1.f-mesh.uv[i];
  auto transform=[&](std::vector<float>& data,bool normal){for(unsigned i=0;i<data.size();i+=3){const float x=data[i],y=data[i+1],z=data[i+2];float out[3];for(unsigned r=0;r<3;r++)out[r]=matrix[r]*x+matrix[4+r]*y+matrix[8+r]*z+(normal?0:matrix[12+r]);
   if(normal){double length=std::sqrt(double(out[0])*out[0]+double(out[1])*out[1]+double(out[2])*out[2]);if(length>0)for(auto& v:out)v=float(v/length);}
   for(unsigned r=0;r<3;r++){need(std::isfinite(out[r]),"Invalid attached geometry");data[i+r]=out[r];}
  }};transform(mesh.positions,false);transform(mesh.normals,true);
 }return geometry;
}
}
