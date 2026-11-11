#include "../../client-core/include/mu/bmd_empty_mesh.hpp"
#include "assets.hpp"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <functional>
namespace mu::assets {
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
 float number(){need(pos+4<=bytes.size(),"Truncated model float");float n;std::memcpy(&n,bytes.data()+pos,4);pos+=4;need(std::isfinite(n)&&std::abs(n)<1e7,"Invalid model float");return n;}
 Vec3 vector(){return {number(),number(),number()};}
 std::string name(){need(pos+32<=bytes.size(),"Truncated model string");auto p=reinterpret_cast<const char*>(bytes.data()+pos);std::string s(p,std::find(p,p+32,'\0'));pos+=32;return s;}
 std::size_t offset()const{return pos;}
};
}
Bytes read_file(const std::filesystem::path& path){
 auto size=std::filesystem::file_size(path);need(size>0&&size<=64*1024*1024,"Asset file size outside supported bounds");
 Bytes b(size);std::ifstream f(path,std::ios::binary);need(bool(f)&&bool(f.read(reinterpret_cast<char*>(b.data()),b.size())),"Could not read complete asset");return b;
}
Bytes decrypt_file(const Bytes& input){
 constexpr std::uint8_t key[16]={0xD1,0x73,0x52,0xF6,0xD2,0x9A,0xCB,0x27,0x3E,0xAF,0x59,0x31,0x37,0xB3,0xE7,0xA2};
 Bytes out(input.size());unsigned previous=0x5E;
 for(std::size_t i=0;i<input.size();i++){out[i]=std::uint8_t((input[i]^key[i%16])-previous);previous=(input[i]+0x3D)&255;}return out;
}
float Terrain::height(unsigned x,unsigned y)const{return heights.at((y%256)*256+(x%256))*0.015f;}
float Terrain::heightAt(double x,double y)const{x=std::clamp(x,0.,254.999);y=std::clamp(y,0.,254.999);unsigned tx=x,ty=y;double u=x-tx,v=y-ty;double left=height(tx,ty)*(1-v)+height(tx,ty+1)*v,right=height(tx+1,ty)*(1-v)+height(tx+1,ty+1)*v;return left*(1-u)+right*u;}
Terrain load_terrain(const std::filesystem::path& data,unsigned serverMap){
 need(serverMap==0||serverMap==3||serverMap==51||serverMap==73||serverMap==74,"Unsupported terrain world");Terrain t;t.world=serverMap+1;auto dir=data/("World"+std::to_string(t.world));
 auto map=read_file(dir/("EncTerrain"+std::to_string(t.world)+".map"));need(map.size()==196610,"Unsupported terrain mapping size");map=decrypt_file(map);need(map[0]==0&&map[1]==t.world,"Unsupported terrain mapping version/world");
 t.layer1=Bytes(map.begin()+2,map.begin()+65538);t.layer2=Bytes(map.begin()+65538,map.begin()+131074);t.alpha=Bytes(map.begin()+131074,map.end());
 unsigned maxTile=serverMap==0?8:13;for(auto n:t.layer1)need(n<=maxTile,"Unknown terrain base tile index");for(auto n:t.layer2)need(n==255||n<=maxTile,"Unknown terrain overlay tile index");
 auto att=decrypt_file(read_file(dir/("EncTerrain"+std::to_string(t.world)+".att")));need((att.size()==65540||att.size()==131076),"Unsupported terrain attribute size");constexpr std::uint8_t mask[3]={0xFC,0xCF,0xAB};
 for(std::size_t i=0;i<att.size();i++)att[i]^=mask[i%3];need(att[0]==0&&att[1]==t.world&&att[2]==255&&att[3]==255,"Invalid terrain attribute header");t.attributes.resize(65536);for(unsigned i=0;i<65536;i++)t.attributes[i]=att.size()==131076?u16(att,4+i*2):att[4+i];
 auto height=read_file(dir/"TerrainHeight.OZB");need(height.size()>=58&&height[4]=='B'&&height[5]=='M',"Unsupported heightmap wrapper");
 need(u32(height,18)==40&&u32(height,22)==256&&u32(height,26)==256&&u16(height,30)==1&&u16(height,32)==8&&u32(height,34)==0,"Heightmap must be an uncompressed 256x256 8-bit bitmap");
 auto off=u32(height,14)+4;need(off>=58&&(off+65536==height.size()||4+1080+65536==height.size()),"Heightmap pixel size mismatch");// Legacy OpenTerrainHeight seeks past the four-byte wrapper and uses
 // a fixed 1080-byte BMP header, rather than bfOffBits (1078 here).
 // Object placements/bridge decks in this Data set align with that +2 shift.
 const std::size_t legacyOffset=4+1080;need(off<=legacyOffset&&legacyOffset-off<=2&&legacyOffset<height.size(),"Unsupported legacy height header");
 t.heights=Bytes(height.begin()+legacyOffset,height.end());t.heights.resize(65536,t.heights.back());return t;
}
Bytes jpeg_payload(const Bytes& wrapped){
 need(wrapped.size()>28&&wrapped[24]==0xFF&&wrapped[25]==0xD8,"Unsupported OZJ wrapper");return Bytes(wrapped.begin()+24,wrapped.end());
}
ObjectFile load_world_objects(const std::filesystem::path& data,unsigned map){
 need(map==0||map==3||map==51||map==73||map==74,"Unsupported object world");unsigned world=map+1;auto dir="World"+std::to_string(world);auto bytes=decrypt_file(read_file(data/dir/("EncTerrain"+std::to_string(world)+".obj")));need(bytes.size()>=4&&bytes[0]==0&&bytes[1]==world&&(bytes.size()-4)%30==0,"Unsupported Lorencia object header/record size");ObjectFile file;file.declaredCount=u16(bytes,2);file.recordCount=(bytes.size()-4)/30;
 // This supplied legacy file declares one extra record. Read only complete
 // records; never fabricate or read past EOF to satisfy its header.
 need(file.recordCount<=10000&&(file.declaredCount==file.recordCount||file.declaredCount==file.recordCount+1),"Object count does not match complete records");Bytes payload(bytes.begin()+4,bytes.end());Reader r(payload);
 for(unsigned i=0;i<file.recordCount;i++){int type=r.small();auto position=r.vector(),rotation=r.vector();float scale=r.number();need(type>=0&&type<4096&&scale>=0&&scale<=100,"Invalid world object type/scale");if(scale==0){need(position.x==0&&position.y==0&&position.z==0,"Invalid zero-scale world object");continue;}need(position.x>=-10000&&position.y>=-10000&&position.x<=40000&&position.y<=40000,"World object placement outside supported range");file.objects.push_back({unsigned(type),position,rotation,scale});}return file;
}
std::string lorencia_model_name(unsigned type){
 struct Range{unsigned first,count;const char* prefix;};static const Range ranges[]={{0,13,"Tree"},{20,8,"Grass"},{30,5,"Stone"},{40,3,"StoneStatue"},{43,1,"SteelStatue"},{44,3,"Tomb"},{50,2,"FireLight"},{52,1,"Bonfire"},{55,1,"DoungeonGate"},{56,2,"MerchantAnimal"},{58,1,"TreasureDrum"},{59,1,"TreasureChest"},{60,1,"Ship"},{65,3,"SteelWall"},{68,1,"SteelDoor"},{69,6,"StoneWall"},{75,4,"StoneMuWall"},{80,1,"Bridge"},{81,4,"Fence"},{85,1,"BridgeStone"},{90,1,"StreetLight"},{91,3,"Cannon"},{95,1,"Curtain"},{96,2,"Sign"},{98,4,"Carriage"},{102,2,"Straw"},{105,1,"Waterspout"},{106,4,"Well"},{110,1,"Hanging"},{111,1,"Stair"},{115,5,"House"},{120,1,"Tent"},{121,6,"HouseWall"},{127,3,"HouseEtc"},{130,3,"Light"},{133,1,"PoseBox"},{140,7,"Furniture"},{150,1,"Candle"},{151,3,"Beer"}};
 for(auto range:ranges)if(type>=range.first&&type<range.first+range.count){unsigned n=type-range.first+1;return std::string(range.prefix)+(n<10?"0":"")+std::to_string(n)+".bmd";}return {};
}
Image decode_tga(const Bytes& wrapped){
 need(wrapped.size()>=22,"Truncated OZT wrapper");std::size_t head=4;
 need(wrapped[head+1]==0&&wrapped[head+2]==2,"Only uncompressed truecolor OZT is supported");
 unsigned width=u16(wrapped,head+12),height=u16(wrapped,head+14),bits=wrapped[head+16];need(width>0&&height>0&&width<=4096&&height<=4096&&(bits==24||bits==32),"Unsupported OZT dimensions/depth");
 unsigned bytes=bits/8;std::size_t offset=head+18+wrapped[head];need(offset+std::size_t(width)*height*bytes<=wrapped.size(),"Truncated OZT pixels");
 Image image{width,height,Bytes(std::size_t(width)*height*4)};bool top=wrapped[head+17]&32,right=wrapped[head+17]&16;
 for(unsigned y=0;y<height;y++)for(unsigned x=0;x<width;x++){
  auto src=offset+(std::size_t(top?y:height-y-1)*width+(right?width-x-1:x))*bytes,dst=(std::size_t(y)*width+x)*4;
  image.rgba[dst]=wrapped[src+2];image.rgba[dst+1]=wrapped[src+1];image.rgba[dst+2]=wrapped[src];image.rgba[dst+3]=bytes==4?wrapped[src+3]:255;
 }return image;
}
Model load_model(const std::filesystem::path& path,const std::vector<unsigned>& retainedActions)try{
 auto b=read_file(path);need(b.size()>=42&&b[0]=='B'&&b[1]=='M'&&b[2]=='D',"Invalid BMD model signature");
 Bytes payload;
 if(b[3]==12){auto size=u32(b,4);need(size==b.size()-8,"BMD encrypted size mismatch");payload=decrypt_file(Bytes(b.begin()+8,b.end()));}
 else{need(b[3]==10,"Unsupported BMD model version");payload=Bytes(b.begin()+4,b.end());}
 Reader r(payload);r.name();int meshes=r.small(),bones=r.small(),actions=r.small();need(meshes>=0&&meshes<=128&&bones>=0&&bones<=256&&actions>=0&&actions<=512,"Invalid BMD model counts");Model model;model.actionCount=actions;
 for(int m=0;m<meshes;m++){
  int vertices=r.small(),normals=r.small(),uvs=r.small(),faces=r.small();int textureIndex=r.small();need(vertices>=0&&vertices<=30000&&normals>=0&&uvs>=0&&faces>=0,"Invalid BMD mesh counts");Mesh mesh;mesh.textureIndex=textureIndex;
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
 for(int a=0;a<actions;a++){int count=r.small();unsigned lock=r.byte();need(count>0&&count<=4096&&lock<=1,"Invalid BMD action");total+=count;need(total<=100000,"BMD animation size too large");keys.push_back(count);model.lockedPositions[a]=lock!=0;if(lock)r.skip(std::size_t(count)*12);}
 std::size_t retainedBones=0;
 for(auto action:retainedActions){if(actions==0&&action==0)continue;need(action<unsigned(actions)&&keys[action]<=256,"Requested model action unavailable or too large");retainedBones+=std::size_t(keys[action])*bones;need(retainedBones<=262144,"Retained animation allocation exceeds bound");model.clips[action].resize(keys[action],std::vector<Bone>(bones));}
 for(int n=0;n<bones;n++){
  Bone bone;auto dummy=r.byte();need(dummy<=1,"Invalid BMD bone flag");bone.dummy=dummy!=0;
  if(!bone.dummy){bone.name=r.name();bone.parent=r.small();need(bone.parent>=-1&&bone.parent<bones&&bone.parent!=n,"Invalid BMD bone parent");
   for(auto& clip:model.clips)for(auto& pose:clip.second)pose[n]=bone;
   for(int a=0;a<actions;a++){
    if(model.clips.count(a)){auto& poses=model.clips.at(a);for(auto& pose:poses)pose[n].position=r.vector();for(auto& pose:poses)pose[n].rotation=r.vector();if(a==0){bone.position=poses[0][n].position;bone.rotation=poses[0][n].rotation;}}
    else r.skip(std::size_t(keys[a])*24);
   }
  }model.bones.push_back(bone);
 }
 need(r.offset()==payload.size(),"Unexpected trailing BMD data");if(model.clips.count(0))model.idlePoses=model.clips.at(0);return model;
}
catch(const std::exception& e){throw std::runtime_error(path.string()+": "+e.what());}
std::vector<Vec3> posed_vertices(const Mesh& mesh,const std::vector<Bone>& bones){
 // Rigid bone-local vertices, Euler radians, parent transforms; first source pose.
 struct Matrix { float r[9];Vec3 p; };std::vector<Matrix> matrices(bones.size());std::vector<unsigned> state(bones.size());
 auto rotate=[](const Matrix& m,Vec3 v){return Vec3{m.r[0]*v.x+m.r[1]*v.y+m.r[2]*v.z,m.r[3]*v.x+m.r[4]*v.y+m.r[5]*v.z,m.r[6]*v.x+m.r[7]*v.y+m.r[8]*v.z};};
 std::function<void(unsigned)> build=[&](unsigned i){need(i<bones.size()&&state[i]!=1,"Cyclic model skeleton");if(state[i]==2)return;state[i]=1;const auto& b=bones[i];
  float x=b.rotation.x,y=b.rotation.y,z=b.rotation.z,cx=std::cos(x),sx=std::sin(x),cy=std::cos(y),sy=std::sin(y),cz=std::cos(z),sz=std::sin(z);
  Matrix local{{cz*cy,cz*sy*sx-sz*cx,cz*sy*cx+sz*sx,sz*cy,sz*sy*sx+cz*cx,sz*sy*cx-cz*sx,-sy,cy*sx,cy*cx},b.position};
  if(b.parent>=0){build(unsigned(b.parent));auto parent=matrices[b.parent],combined=local;
   for(int row=0;row<3;row++)for(int col=0;col<3;col++){combined.r[row*3+col]=0;for(int k=0;k<3;k++)combined.r[row*3+col]+=parent.r[row*3+k]*local.r[k*3+col];}
   auto p=rotate(parent,local.p);combined.p={p.x+parent.p.x,p.y+parent.p.y,p.z+parent.p.z};local=combined;
  }matrices[i]=local;state[i]=2;
 };
 for(unsigned i=0;i<bones.size();i++)build(i);std::vector<Vec3> out;
 for(auto& v:mesh.vertices){if(v.bone==-1){out.push_back(v.position);continue;}need(v.bone>=0&&unsigned(v.bone)<bones.size(),"Pose bone index out of bounds");auto& m=matrices[v.bone];auto p=rotate(m,v.position);out.push_back({p.x+m.p.x,p.y+m.p.y,p.z+m.p.z});}return out;
}
}
