#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include <array>
#include <map>
namespace mu::assets {
using Bytes=std::vector<std::uint8_t>;
struct Image { unsigned width=0,height=0; Bytes rgba; };
struct Terrain {
 unsigned world=0;
 Bytes heights,layer1,layer2,alpha;
 std::vector<std::uint16_t> attributes;
 float height(unsigned x,unsigned y) const;
 float heightAt(double x,double y) const;
};
struct Vec3 { float x,y,z; };
struct WorldObject { unsigned type;Vec3 position,rotation;float scale; };
struct ObjectFile { unsigned declaredCount=0,recordCount=0;std::vector<WorldObject> objects; };
ObjectFile load_world_objects(const std::filesystem::path& data,unsigned serverMap);
std::string lorencia_model_name(unsigned type);
struct Vertex { int bone; Vec3 position; };
struct Face { std::array<unsigned,3> vertex,uv;std::array<unsigned,3> normal{}; };
struct Mesh { int textureIndex=0;std::string texture; std::vector<Vertex> vertices,normals; std::vector<std::array<float,2>> uv; std::vector<Face> faces; };
struct Bone { int parent=-1; bool dummy=true; std::string name; Vec3 position{},rotation{}; };
struct Model { unsigned actionCount=0; std::map<unsigned,bool> lockedPositions; std::vector<Mesh> meshes; std::vector<Bone> bones; std::vector<std::vector<Bone>> idlePoses; std::map<unsigned,std::vector<std::vector<Bone>>> clips; };
Bytes read_file(const std::filesystem::path&);
Bytes decrypt_file(const Bytes&);
Terrain load_terrain(const std::filesystem::path& data,unsigned serverMap);
Image decode_tga(const Bytes&);
Bytes jpeg_payload(const Bytes&);
Model load_model(const std::filesystem::path&,const std::vector<unsigned>& retainedActions={0});
std::vector<Vec3> posed_vertices(const Mesh&,const std::vector<Bone>&);
}
