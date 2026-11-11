#pragma once
#include "bmd_model.hpp"
#include <functional>
namespace mu::core::bmd {
struct GeometryMesh {std::string texture;int textureIndex;std::vector<float> positions,normals,uv;};
struct Geometry {std::vector<GeometryMesh> meshes;};
// Rigid bone-local BMD geometry in native coordinate system, initial action-0 pose.
inline Geometry geometry_from_pose(const std::vector<Mesh>& meshes,const std::vector<Bone>& bones){
 struct Matrix{float r[9];Vec3 p;};std::vector<Matrix> matrices(bones.size());std::vector<unsigned> state(bones.size());
 auto rotate=[](const Matrix& m,Vec3 v){return Vec3{m.r[0]*v.x+m.r[1]*v.y+m.r[2]*v.z,m.r[3]*v.x+m.r[4]*v.y+m.r[5]*v.z,m.r[6]*v.x+m.r[7]*v.y+m.r[8]*v.z};};
 std::function<void(unsigned)> build=[&](unsigned i){
  need(i<bones.size()&&state[i]!=1,"Invalid geometry hierarchy");if(state[i]==2)return;state[i]=1;const auto& b=bones[i];
  float x=b.rotation.x,y=b.rotation.y,z=b.rotation.z,cx=std::cos(x),sx=std::sin(x),cy=std::cos(y),sy=std::sin(y),cz=std::cos(z),sz=std::sin(z);
  Matrix local{{cz*cy,cz*sy*sx-sz*cx,cz*sy*cx+sz*sx,sz*cy,sz*sy*sx+cz*cx,sz*sy*cx-cz*sx,-sy,cy*sx,cy*cx},b.position};
  if(b.parent>=0){build(unsigned(b.parent));const auto parent=matrices[unsigned(b.parent)];auto combined=local;
   for(int row=0;row<3;row++)for(int col=0;col<3;col++){combined.r[row*3+col]=0;for(int k=0;k<3;k++)combined.r[row*3+col]+=parent.r[row*3+k]*local.r[k*3+col];}
   auto p=rotate(parent,local.p);combined.p={p.x+parent.p.x,p.y+parent.p.y,p.z+parent.p.z};local=combined;
  }matrices[i]=local;state[i]=2;
 };
 for(unsigned i=0;i<bones.size();i++)build(i);
 auto transform=[&](const Vertex& v,bool normal){
  auto p=v.position;if(v.bone!=-1){need(v.bone>=0&&unsigned(v.bone)<matrices.size(),"Geometry bone out of bounds");const auto& m=matrices[unsigned(v.bone)];p=rotate(m,p);if(!normal)p={p.x+m.p.x,p.y+m.p.y,p.z+m.p.z};}
  need(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z),"Invalid transformed geometry");
  if(normal){double length=std::sqrt(double(p.x)*p.x+double(p.y)*p.y+double(p.z)*p.z);if(length>0)p={float(p.x/length),float(p.y/length),float(p.z/length)};}
  return p;
 };
 Geometry result;std::size_t corners=0;
 for(const auto& mesh:meshes){
  corners+=mesh.faces.size()*3;need(corners<=262144,"Geometry output exceeds corner cap");
  GeometryMesh out{mesh.texture,mesh.textureIndex,{},{},{}};out.positions.reserve(mesh.faces.size()*9);out.normals.reserve(mesh.faces.size()*9);out.uv.reserve(mesh.faces.size()*6);
  for(const auto& face:mesh.faces)for(unsigned i=0;i<3;i++){
   auto p=transform(mesh.vertices.at(face.vertex[i]),false),n=transform(mesh.normals.at(face.normal[i]),true);auto uv=mesh.uv.at(face.uv[i]);
   out.positions.insert(out.positions.end(),{p.x,p.y,p.z});out.normals.insert(out.normals.end(),{n.x,n.y,n.z});out.uv.insert(out.uv.end(),{uv[0],uv[1]});
  }result.meshes.push_back(std::move(out));
 }
 return result;
}
inline Geometry initial_geometry(const Model& model){return geometry_from_pose(model.meshes,model.bones);}
inline Geometry bound_geometry(const Model& equipment,const Model& skeleton,unsigned action,unsigned frame){
 auto clip=skeleton.clips.find(action);need(clip!=skeleton.clips.end()&&frame<clip->second.size(),"Player pose unavailable");
 for(const auto& mesh:equipment.meshes){
  auto check=[&](int index,bool allowStatic){
   if(allowStatic&&index==-1)return;
   need(index>=0&&unsigned(index)<equipment.bones.size()&&unsigned(index)<skeleton.bones.size(),"Equipment bone outside Player skeleton");
   const auto& a=equipment.bones[unsigned(index)];const auto& b=skeleton.bones[unsigned(index)];
   need(a.name==b.name&&a.parent==b.parent&&a.dummy==b.dummy,"Equipment Player bone binding mismatch");
  };
  for(const auto& v:mesh.vertices)check(v.bone,false);
  for(const auto& n:mesh.normals)check(n.bone,true);
 }
 return geometry_from_pose(equipment.meshes,clip->second.at(frame));
}

}
