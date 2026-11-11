#pragma once
#include <cstddef>
#include <vector>
namespace mu::bmd_validation {
// Some original Data models retain an exporter placeholder: one zero vertex,
// one zero normal, both node -8888, and faces referencing only that point.
// These triangles have no area in any pose. Keep the mesh slot/texture ordinal
// but never evaluate its invalid bone. This is NOT a fallback for damaged skin.
template<class Mesh> bool discard_empty_sentinel_mesh(Mesh& mesh){
 if(mesh.vertices.size()!=1||mesh.normals.size()!=1||mesh.faces.empty())return false;
 const auto& v=mesh.vertices[0];const auto& n=mesh.normals[0];
 if(v.bone!=-8888||n.bone!=-8888||v.position.x!=0||v.position.y!=0||v.position.z!=0||n.position.x!=0||n.position.y!=0||n.position.z!=0)return false;
 for(const auto& face:mesh.faces)for(unsigned i=0;i<3;i++)if(face.vertex[i]!=0||face.normal[i]!=0)return false;
 mesh.faces.clear();mesh.vertices.clear();mesh.normals.clear();return true;
}
// Elbeland's water model mixes the same exporter placeholder with live faces.
// Discard only triangles whose three vertex AND normal indices each reference
// one zero/-8888 record. Never discard a live triangle to hide a bad bone.
template<class Mesh> std::size_t discard_empty_sentinel_faces(Mesh& mesh){
 const auto original=mesh.faces.size();
 if(discard_empty_sentinel_mesh(mesh))return original;
 const auto zero=[](const auto& v){return v.bone==-8888&&v.position.x==0&&v.position.y==0&&v.position.z==0;};
 std::vector<bool> removedVertex(mesh.vertices.size()),removedNormal(mesh.normals.size());
 auto kept=mesh.faces;kept.clear();
 for(const auto& face:mesh.faces){
  const auto v=face.vertex[0],n=face.normal[0];
  const bool empty=v<mesh.vertices.size()&&n<mesh.normals.size()&&
   face.vertex[1]==v&&face.vertex[2]==v&&face.normal[1]==n&&face.normal[2]==n&&
   zero(mesh.vertices[v])&&zero(mesh.normals[n]);
  if(empty){removedVertex[v]=true;removedNormal[n]=true;}else kept.push_back(face);
 }
 const auto count=original-kept.size();if(!count)return 0;
 mesh.faces.swap(kept);
 for(const auto& face:mesh.faces)for(unsigned i=0;i<3;i++){
  if(face.vertex[i]<removedVertex.size())removedVertex[face.vertex[i]]=false;
  if(face.normal[i]<removedNormal.size())removedNormal[face.normal[i]]=false;
 }
 // Keep indices stable. Only records proven unused after pruning become static;
 // any sentinel still used by a live face remains invalid for the strict parser.
 for(std::size_t i=0;i<removedVertex.size();i++)if(removedVertex[i])mesh.vertices[i].bone=-1;
 for(std::size_t i=0;i<removedNormal.size();i++)if(removedNormal[i])mesh.normals[i].bone=-1;
 return count;
}
}
