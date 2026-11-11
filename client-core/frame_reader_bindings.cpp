#include "mu/frame_reader.hpp"
#include "mu/connect_protocol.hpp"
#include "mu/game_profile.hpp"
#include "mu/client_identity.hpp"
#include "mu/simple_modulus.hpp"
#include "mu/account_protocol.hpp"
#include "mu/character_protocol.hpp"
#include "mu/appearance.hpp"
#include "mu/companions.hpp"
#include "mu/item_resources.hpp"
#include "mu/armor_resources.hpp"
#include "mu/equipment_grip.hpp"
#include "mu/equipment_actions.hpp"
#include "mu/attachment_geometry.hpp"
#include "mu/animation_pose.hpp"
#include "mu/motion_animation.hpp"
#include "mu/bmd_model.hpp"
#include "mu/bmd_geometry.hpp"
#include "mu/texture.hpp"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <stdexcept>
#include <optional>
#include <memory>

namespace {
using emscripten::val;
mu::core::WireBytes inputBytes(const val& input) {
 if(!input.instanceof(val::global("Uint8Array")))throw std::runtime_error("Expected Uint8Array");
 auto size=input["byteLength"].as<unsigned>();
 if(size>mu::core::FrameReader::max_buffer)throw std::runtime_error("Input exceeds byte bound");
 mu::core::WireBytes bytes(size);
 if(size)val(emscripten::typed_memory_view(size,bytes.data())).call<void>("set",input);
 return bytes;
}
val copyBytes(const mu::core::WireBytes& bytes) {
 return val::global("Uint8Array").new_(val(emscripten::typed_memory_view(bytes.size(),bytes.data())));
}
mu::core::bmd::Bytes modelInput(const val& input){
 if(!input.instanceof(val::global("Uint8Array")))throw std::runtime_error("Expected BMD Uint8Array");
 unsigned size=input["byteLength"].as<unsigned>();if(size>16*1024*1024)throw std::runtime_error("BMD input exceeds byte cap");
 mu::core::bmd::Bytes bytes(size);if(size)val(emscripten::typed_memory_view(size,bytes.data())).call<void>("set",input);return bytes;
}
val textureResult(const std::string& format,const val& input){auto result=val::object();
 try{auto bytes=modelInput(input);mu::core::TextureData texture;
 if(format=="OZT")texture=mu::core::decode_ozt(bytes);else if(format=="OZJ")texture=mu::core::unwrap_ozj(bytes);else throw std::runtime_error("Unsupported texture wrapper");
 result.set("width",texture.width);result.set("height",texture.height);result.set("jpeg",texture.jpeg);result.set("bytes",copyBytes(texture.bytes));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val geometryValue(const mu::core::bmd::Geometry& geometry){
 auto meshes=val::array();
 auto floats=[](const std::vector<float>& data){return val::global("Float32Array").new_(val(emscripten::typed_memory_view(data.size(),data.data())));};
 for(const auto& mesh:geometry.meshes){auto row=val::object();row.set("texture",mesh.texture);row.set("textureIndex",mesh.textureIndex);row.set("positions",floats(mesh.positions));row.set("normals",floats(mesh.normals));row.set("uv",floats(mesh.uv));meshes.call<void>("push",row);}
 return meshes;
}
class ModelAnimation {
 mu::core::bmd::Model model;
 public:ModelAnimation(const val& bytes):model(mu::core::bmd::decode_model(modelInput(bytes))){}
 val sampleResult(double seconds){auto result=val::object();try{auto pose=mu::core::bmd::sample_pose(model,0,seconds);result.set("meshes",geometryValue(mu::core::bmd::geometry_from_pose(model.meshes,pose)));result.set("ok",true);}catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;}
 val sampleBonesResult(double seconds,const val& requested){auto result=val::object();try{if(!val::global("Array").call<bool>("isArray",requested))throw std::runtime_error("Expected bone index array");unsigned count=requested["length"].as<unsigned>();if(count>64)throw std::runtime_error("Bone request cap exceeded");auto pose=mu::core::bmd::sample_pose(model,0,seconds);auto matrices=val::array();for(unsigned i=0;i<count;i++){double index=requested[i].as<double>();if(!std::isfinite(index)||index<0||std::floor(index)!=index||index>=pose.size())throw std::runtime_error("Requested bone out of bounds");auto matrix=mu::core::bmd::bone_world_matrix(pose,unsigned(index));auto row=val::array();for(float n:matrix)row.call<void>("push",n);matrices.call<void>("push",row);}result.set("matrices",matrices);result.set("ok",true);}catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;}

};
// Generic multipart actor rig: all parts share the primary action-0 skeleton.
class ActorAnimation {
 unsigned action;mu::core::bmd::Model skeleton;std::vector<mu::core::bmd::Model> parts;
 public:
 ActorAnimation(const val& bytes):ActorAnimation(bytes,0){}
 ActorAnimation(const val& bytes,unsigned selected):action(selected),skeleton(mu::core::bmd::decode_model(modelInput(bytes),{selected})){if(skeleton.lockedPositions.at(action))mu::core::bmd::normalize_motion_roots(skeleton,{action},action);parts.push_back(skeleton);}
 void configureMonster(const val& bytes){auto meta=mu::core::bmd::decode_model(modelInput(bytes),{});std::vector<unsigned> clips;for(auto a:{0u,2u,3u,4u,5u,6u})if(a<meta.actionCount)clips.push_back(a);auto full=mu::core::bmd::decode_model(modelInput(bytes),clips);for(auto selected:clips)if(full.lockedPositions.at(selected)){const auto origin=full.clips.at(selected).front();for(auto& frame:full.clips.at(selected))for(unsigned i=0;i<frame.size();i++)if(frame[i].parent<0){frame[i].position.x=origin[i].position.x;frame[i].position.y=origin[i].position.y;}}skeleton=std::move(full);parts[0]=skeleton;}
 void configureMount(const val& bytes){auto meta=mu::core::bmd::decode_model(modelInput(bytes),{});std::vector<unsigned> clips;for(unsigned a=0;a<meta.actionCount&&a<16;a++)clips.push_back(a);skeleton=mu::core::bmd::decode_model(modelInput(bytes),clips);for(auto selected:clips)if(skeleton.lockedPositions.at(selected))mu::core::bmd::normalize_motion_roots(skeleton,{selected},0);parts[0]=skeleton;}
 void configureCharacter(const val& bytes,unsigned idle,unsigned walk,unsigned attack,unsigned death){skeleton=mu::core::bmd::decode_model(modelInput(bytes),{idle,walk,attack,death});action=idle;parts[0]=skeleton;}
 unsigned actionFrames(unsigned selected){auto i=skeleton.clips.find(selected);if(i==skeleton.clips.end())return 0;return unsigned(i->second.size());}
 val sampleActionOnceResult(double seconds,unsigned selected){unsigned frames=actionFrames(selected);if(frames){const auto lock=skeleton.lockedPositions.find(selected);if(lock!=skeleton.lockedPositions.end()&&lock->second&&frames>1)frames--;seconds=std::min(seconds,double(frames-1)/6.25);}return sampleActionResult(std::max(0.,seconds),selected);}
 val sampleActionResult(double seconds,unsigned selected){action=selected;return sampleResult(seconds);}
 void addPart(const val& bytes){if(parts.size()>=16)throw std::runtime_error("Actor part cap exceeded");auto part=mu::core::bmd::decode_model(modelInput(bytes),{});mu::core::bmd::bound_geometry(part,skeleton,action,0);parts.push_back(std::move(part));}
 val sampleResult(double seconds){auto result=val::object();try{auto pose=mu::core::bmd::sample_pose(skeleton,action,seconds);unsigned period=unsigned(skeleton.clips.at(action).size());if(skeleton.lockedPositions.at(action)&&period>1)period--;result.set("frame",std::fmod(seconds*6.25,double(std::max(1u,period))));mu::core::bmd::Geometry geometry;std::size_t corners=0;for(const auto& part:parts){auto next=mu::core::bmd::geometry_from_pose(part.meshes,pose);for(auto& mesh:next.meshes){corners+=mesh.positions.size()/3;if(corners>262144)throw std::runtime_error("Actor corner cap exceeded");geometry.meshes.push_back(std::move(mesh));}}result.set("meshes",geometryValue(geometry));result.set("ok",true);}catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;}
};
class BodyAnimation {
 mu::core::bmd::Model player;unsigned action;bool rfSatan=false;std::vector<mu::core::bmd::Model> parts;struct Attachment{mu::core::bmd::Model model;unsigned slot,index;bool safe;};std::vector<Attachment> weapons;std::unique_ptr<mu::core::bmd::MotionAnimation> motion;
 public:
 BodyAnimation(const val& bytes,unsigned selected):player(mu::core::bmd::decode_model(modelInput(bytes),{selected})),action(selected){}
 void configureHelper(unsigned cls,unsigned armor){rfSatan=cls==6&&(armor==59||armor==60||armor==61||armor==73||armor==83);}
 void addPart(const val& bytes){if(parts.size()>=5)throw std::runtime_error("Animation requires at most five body parts");auto model=mu::core::bmd::decode_model(modelInput(bytes),{});mu::core::bmd::bound_geometry(model,player,action,0);parts.push_back(std::move(model));}
 void addWeapon(const val& bytes,unsigned hand,unsigned item,bool zone){if(weapons.size()>=4)throw std::runtime_error("Too many attachments");for(const auto& weapon:weapons)if(weapon.slot==hand)throw std::runtime_error("Duplicate attachment slot");auto model=mu::core::bmd::decode_model(modelInput(bytes));mu::core::bmd::attachment_geometry(model,player,action,0,hand,item,zone);weapons.push_back({std::move(model),hand,item,zone});}
 void setWeapon(const val& bytes,unsigned hand,unsigned item,bool zone){auto model=mu::core::bmd::decode_model(modelInput(bytes));mu::core::bmd::attachment_geometry(model,player,action,0,hand,item,zone);std::vector<Attachment> next;next.push_back({std::move(model),hand,item,zone});weapons.swap(next);}
 void configureSpell(const val& bytes,unsigned walk,unsigned run,unsigned reference,unsigned attack){auto next=mu::core::bmd::decode_model(modelInput(bytes),{action,walk,run,reference,attack,146,147,11,34});mu::core::bmd::normalize_motion_roots(next,{action,walk,run,attack,146,147,11,34},reference);player=std::move(next);motion=std::make_unique<mu::core::bmd::MotionAnimation>(player,std::vector<unsigned>{action,walk,run,attack,146,147,11,34});}
 void configureCombat(const val& bytes,unsigned walk,unsigned run,unsigned reference,unsigned attack){auto next=mu::core::bmd::decode_model(modelInput(bytes),{action,walk,run,reference,attack});mu::core::bmd::normalize_motion_roots(next,{action,walk,run,attack},reference);player=std::move(next);motion=std::make_unique<mu::core::bmd::MotionAnimation>(player,std::vector<unsigned>{action,walk,run,attack});}
 void setSafeZone(bool safe){for(auto& weapon:weapons)weapon.safe=safe;}
 void configureWorld(const val& bytes,unsigned safeIdle,unsigned safeWalk,unsigned idle,unsigned walk,unsigned run,unsigned attack){auto next=mu::core::bmd::decode_model(modelInput(bytes),{safeIdle,safeWalk,idle,walk,run,attack,146,147,11,34,233,235,239,240,237,238,231,232});mu::core::bmd::normalize_motion_roots(next,{safeIdle,safeWalk,idle,walk,run,attack,146,147,11,34},safeIdle);player=std::move(next);motion=std::make_unique<mu::core::bmd::MotionAnimation>(player,std::vector<unsigned>{safeIdle,safeWalk,run,attack,idle,walk,146,147,11,34,233,235,239,240,237,238,231,232},true);}
 void configureMountedWorld(const val& bytes,unsigned safeIdle,unsigned safeWalk,unsigned idle,unsigned walk,unsigned run,unsigned attack){configureWorld(bytes,safeIdle,safeWalk,idle,walk,run,attack);motion=std::make_unique<mu::core::bmd::MotionAnimation>(player,std::vector<unsigned>{safeIdle,safeWalk,run,attack,idle,walk,attack,attack,idle,run,233,235,239,240,237,238,231,232},true);}
 void configureMotion(const val& bytes,unsigned walk,unsigned run,unsigned reference){auto next=mu::core::bmd::decode_model(modelInput(bytes),{action,walk,run,reference});mu::core::bmd::normalize_motion_roots(next,{action,walk,run},reference);player=std::move(next);motion=std::make_unique<mu::core::bmd::MotionAnimation>(player,std::vector<unsigned>{action,walk,run});}
 val sampleMotionResult(double seconds,unsigned mode){return sample(seconds,mode,true);}
 val sampleResult(double seconds){return sample(seconds,0,false);}
 val sample(double seconds,unsigned mode,bool moving){auto result=val::object();try{if(parts.size()!=5)throw std::runtime_error("Incomplete animated body");if(moving&&!motion)throw std::runtime_error("Motion not configured");auto pose=moving?motion->sample(seconds,mode):mu::core::bmd::sample_pose(player,action,seconds);if(moving){result.set("frame",motion->frame());result.set("mode",motion->currentMode());}mu::core::bmd::Model rig;rig.bones=pose;rig.clips[action]={pose};mu::core::bmd::Geometry geometry;std::size_t corners=0;
 auto append=[&](mu::core::bmd::Geometry source){for(auto& mesh:source.meshes){corners+=mesh.positions.size()/3;if(corners>262144)throw std::runtime_error("Animated body exceeds corner cap");geometry.meshes.push_back(std::move(mesh));}};
 for(const auto& part:parts)append(mu::core::bmd::geometry_from_pose(part.meshes,pose));for(const auto& weapon:weapons)append(mu::core::bmd::attachment_geometry(weapon.model,rig,action,0,weapon.slot,weapon.index,weapon.safe,seconds,moving&&(mode==8||mode==9),rfSatan));for(const auto& weapon:weapons)if(weapon.slot==7&&(weapon.index==13*512+30||weapon.index==12*512+40||weapon.index==12*512+49||weapon.index==12*512+50)){auto bones=val::object();for(unsigned id:{2u,17u,19u}){auto matrix=mu::core::bmd::bone_world_matrix(pose,id);auto values=val::array();for(float n:matrix)values.call<void>("push",n);bones.set(std::to_string(id),values);}result.set("clothBones",bones);}
 auto smokeBones=val::array();auto smokeMatrix=mu::core::bmd::bone_world_matrix(pose,0);for(float n:smokeMatrix)smokeBones.call<void>("push",n);result.set("equipmentRoot",smokeBones);
 // Right upper arm used by RenderGuild (ZzzCharacter.cpp BoneTransform[26]).
 auto guildMatrix=mu::core::bmd::bone_world_matrix(pose,26);auto guildBone=val::array();for(float n:guildMatrix)guildBone.call<void>("push",n);result.set("guildBone",guildBone);
 for(const auto& weapon:weapons)if(weapon.slot==8&&weapon.index==13*512+1){auto matrix=mu::core::bmd::bone_world_matrix(pose,34);float offset[3]={20.f,rfSatan?-5.f:0.f,rfSatan?35.f:15.f};auto point=val::array();for(unsigned r=0;r<3;r++)point.call<void>("push",matrix[r]*offset[0]+matrix[4+r]*offset[1]+matrix[8+r]*offset[2]+matrix[12+r]);result.set("helperLight",point);}
 result.set("meshes",geometryValue(geometry));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;}
};
val boneNamesResult(const val& input){auto result=val::object();
 try{auto model=mu::core::bmd::decode_model(modelInput(input),{});auto names=val::array();for(const auto& bone:model.bones)names.call<void>("push",bone.dummy?std::string("dummy"):bone.name);result.set("names",names);result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val bmdGeometryResult(const val& input){auto result=val::object();
 try{auto geometry=mu::core::bmd::initial_geometry(mu::core::bmd::decode_model(modelInput(input)));result.set("meshes",geometryValue(geometry));result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val attachmentGeometryResult(const val& itemBytes,const val& playerBytes,unsigned action,unsigned frame,unsigned slot,unsigned index,bool safe,bool rfSatan){auto result=val::object();
 try{if(action>=512||frame>=256)throw std::runtime_error("Invalid attachment pose selection");
 auto item=mu::core::bmd::decode_model(modelInput(itemBytes));auto player=mu::core::bmd::decode_model(modelInput(playerBytes),{action});
 result.set("meshes",geometryValue(mu::core::bmd::attachment_geometry(item,player,action,frame,slot,index,safe,0,false,rfSatan)));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val boundGeometryResult(const val& equipmentBytes,const val& skeletonBytes,unsigned action,unsigned frame){auto result=val::object();
 try{if(action>=512||frame>=256)throw std::runtime_error("Invalid Player pose selection");
 auto equipment=mu::core::bmd::decode_model(modelInput(equipmentBytes),{});
 auto skeleton=mu::core::bmd::decode_model(modelInput(skeletonBytes),{action});
 auto geometry=mu::core::bmd::bound_geometry(equipment,skeleton,action,frame);result.set("meshes",geometryValue(geometry));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val inventoryArmorGeometryResult(const val& equipmentBytes,const val& skeletonBytes,unsigned index){auto result=val::object();
 try{if(index!=4155&&index!=4156&&index!=4157&&index!=4169&&index!=4179)throw std::runtime_error("Not a native inventory armor substitution");
 auto equipment=mu::core::bmd::decode_model(modelInput(equipmentBytes),{});
 auto skeleton=mu::core::bmd::decode_model(modelInput(skeletonBytes),{0});
 // RenderObjectScreen uses MODEL_PLAYER's BoneTransform for MODEL_ARMORINVEN_*.
 // These dedicated inventory files have different bone names; native binding is
 // by index. Keep the normal equipment name/parent validation unchanged.
 auto clip=skeleton.clips.find(0);if(clip==skeleton.clips.end()||clip->second.empty())throw std::runtime_error("Player inventory pose unavailable");
 for(const auto& mesh:equipment.meshes){for(const auto& v:mesh.vertices)if(v.bone<0||unsigned(v.bone)>=skeleton.bones.size())throw std::runtime_error("Inventory vertex outside Player skeleton");for(const auto& n:mesh.normals)if(n.bone < -1||(n.bone>=0&&unsigned(n.bone)>=skeleton.bones.size()))throw std::runtime_error("Inventory normal outside Player skeleton");}
 result.set("meshes",geometryValue(mu::core::bmd::geometry_from_pose(equipment.meshes,clip->second.at(0))));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val bmdModelResult(const val& input){auto result=val::object();
 try{
  auto bytes=modelInput(input);
  auto model=mu::core::bmd::decode_model(bytes);auto meshes=val::array(),parents=val::array(),clips=val::array();
  for(const auto& mesh:model.meshes){auto row=val::object();row.set("vertices",mesh.vertices.size());row.set("normals",mesh.normals.size());row.set("uvs",mesh.uv.size());row.set("triangles",mesh.faces.size());row.set("textureIndex",mesh.textureIndex);row.set("texture",mesh.texture);meshes.call<void>("push",row);}
  for(const auto& bone:model.bones)parents.call<void>("push",bone.parent);
  for(const auto& clip:model.clips){auto row=val::object();row.set("action",clip.first);row.set("frames",clip.second.size());auto locked=model.lockedPositions.find(clip.first);row.set("period",clip.second.size()-(locked!=model.lockedPositions.end()&&locked->second&&clip.second.size()>1?1:0));clips.call<void>("push",row);}
  result.set("meshes",meshes);result.set("boneParents",parents);result.set("actions",model.actionCount);result.set("retainedClips",clips);result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val equipmentActionsResult(unsigned cls,int right,int left,const val& itemTable){auto result=val::object();
 try{bool two=right>=0?mu::core::item_two_hand(modelInput(itemTable),unsigned(right)):false;auto actions=mu::core::equipped_actions(cls,right,left,two);result.set("idle",actions[0]);result.set("walk",actions[1]);result.set("run",actions[2]);result.set("twoHand",two);result.set("attack",mu::core::equipped_attack(right,left,two));result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val equipmentGripResult(unsigned slot,unsigned index,bool safe){auto result=val::object();
 try{auto grip=mu::core::equipment_grip(slot,index,safe);auto matrix=val::array();for(float v:grip.transform)matrix.call<void>("push",v);result.set("bone",grip.bone);result.set("transform",matrix);result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val armorResourceResult(unsigned part,unsigned type,unsigned cls,unsigned tier){auto result=val::object();
 try{result.set("path","Player/"+mu::core::armor_model(part,type,cls,tier));result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val itemResourceResult(unsigned index,unsigned level){auto result=val::object();
 try{auto path=mu::core::builtin_item_model(index,level);result.set("path",path);result.set("mapped",!path.empty());result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val companionsResult(const val& input){
 auto result=val::object();
 try{auto c=mu::core::decode_companions(inputBytes(input));
 auto convert=[](const mu::core::Companion& p){auto row=val::object();row.set("status",p.status);row.set("index",p.index);row.set("variant",p.variant);row.set("customIndex",p.customIndex);return row;};
 result.set("wing",convert(c.wing));result.set("pet",convert(c.pet));result.set("raw",copyBytes(c.raw));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val appearanceResult(const val& input) {
 auto result=val::object();
 try{auto a=mu::core::decode_appearance(inputBytes(input));auto parts=val::array();
 for(const auto& p:a.parts){auto row=val::object();row.set("slot",p.slot);row.set("index",p.index);row.set("visualLevel",p.visualLevel);row.set("present",p.present);row.set("excellent",p.excellent);row.set("ancient",p.ancient);parts.call<void>("push",row);}
 result.set("baseClass",a.baseClass);result.set("tier",a.tier);result.set("parts",parts);result.set("raw",copyBytes(a.raw));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val characterCodecResult(const std::string& operation,const val& input) {
 auto result=val::object();
 try{
  if(operation=="request")result.set("bytes",copyBytes(mu::core::character_list_request()));
  else if(operation=="roster"){
   auto roster=mu::core::character_roster(inputBytes(input));auto rows=val::array();
   for(const auto& c:roster.characters){auto row=val::object();row.set("slot",c.slot);row.set("level",c.level);row.set("control",c.control);row.set("guildStatus",c.guildStatus);row.set("name",c.name);row.set("charset",copyBytes(c.charset));rows.call<void>("push",row);}
   result.set("characters",rows);result.set("classCode",roster.classCode);result.set("moveCount",roster.moveCount);result.set("extWarehouse",roster.extWarehouse);
  }else if(operation=="policy"){
   auto p=mu::core::character_policy(inputBytes(input));result.set("flag",p.flag);result.set("classCode",p.classCode);result.set("experienceA",p.experienceA);result.set("experienceB",p.experienceB);result.set("maxLevel",p.maxLevel);result.set("deleteMaxLevel",p.deleteMaxLevel);
  }else throw std::runtime_error("Unknown character codec operation");
  result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val loginPacketResult(const std::string& account,const std::string& password,std::uint32_t tick,const val& version,const std::string& serial) {
 auto result=val::object();
 try{result.set("bytes",copyBytes(mu::core::account_login_packet(account,password,tick,inputBytes(version),serial)));result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val loginResponseResult(const val& bytes) {
 auto result=val::object();
 try{auto code=mu::core::account_login_result(inputBytes(bytes));result.set("code",code);result.set("accepted",code==1);result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val transformResult(const val& input,unsigned key1,unsigned key2,bool mhp,unsigned mhp1,unsigned mhp2,bool encode) {
 auto result=val::object();
 try {
  if(key1>255||key2>255||mhp1>255||mhp2>255)throw std::runtime_error("Transport key outside byte range");
  mu::core::TransportKeys keys{std::uint8_t(key1),std::uint8_t(key2),mhp,std::uint8_t(mhp1),std::uint8_t(mhp2)};
  auto inputData=inputBytes(input);auto output=encode?mu::core::encode_transport(std::move(inputData),keys):mu::core::decode_transport(std::move(inputData),keys);
  result.set("bytes",copyBytes(output));result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}
 return result;
}
val connectCodecResult(const std::string& operation,const val& input,unsigned code) {
 auto result=val::object();
 try {
  if(operation=="list-request")result.set("bytes",copyBytes(mu::core::server_list_request()));
  else if(operation=="endpoint-request")result.set("bytes",copyBytes(mu::core::server_endpoint_request(code)));
  else if(operation=="handshake")mu::core::connect_handshake(inputBytes(input));
  else if(operation=="list") {
   auto list=val::array();
   for(const auto& entry:mu::core::server_list(inputBytes(input))){auto row=val::object();row.set("code",entry.code);row.set("load",entry.load);row.set("type",entry.type);list.call<void>("push",row);}
   result.set("servers",list);
  }else if(operation=="endpoint"){
   auto endpoint=mu::core::server_endpoint(inputBytes(input));auto row=val::object();row.set("address",endpoint.address);row.set("port",endpoint.port);result.set("endpoint",row);
  }else throw std::runtime_error("Unknown ConnectServer operation");
  result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}
 return result;
}
val deriveProfileResult(const std::string& serial,unsigned raw1,unsigned raw2) {
 auto result=val::object();
 try {
  auto keys=mu::core::derive_transport_keys(serial,raw1,raw2);auto data=val::object();
  data.set("key1",keys.key1);data.set("key2",keys.key2);data.set("mhp",keys.mhp);data.set("mhp1",keys.mhp1);data.set("mhp2",keys.mhp2);
  result.set("keys",data);result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val gameGreetingResult(const val& input) {
 auto result=val::object();
 try {
  auto greeting=mu::core::game_greeting(inputBytes(input));auto data=val::object();auto version=val::array();
  for(auto b:greeting.encodedVersion)version.call<void>("push",unsigned(b));
  data.set("index",greeting.index);data.set("serverCode",greeting.serverCode);data.set("encodedVersion",version);
  result.set("greeting",data);result.set("ok",true);
 }catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val identityPacketResult(const std::string& identity) {
 auto result=val::object();
 try {result.set("bytes",copyBytes(mu::core::hardware_identity_packet(identity)));result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val packetFilterResult(const val& bytes,bool encode) {
 auto result=val::object();
 try {result.set("bytes",copyBytes(mu::core::filter_plain_packet(inputBytes(bytes),encode)));result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
val validateKeyFileResult(const val& bytes) {
 auto result=val::object();
 try {mu::core::parse_modulus_key(inputBytes(bytes));result.set("ok",true);}
 catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
}
class BrowserPacketCipher {
 mu::core::PacketCipher cipher;
public:
 BrowserPacketCipher(val enc,val dec):cipher(mu::core::parse_modulus_key(inputBytes(enc)),mu::core::parse_modulus_key(inputBytes(dec))){}
 val encodeResult(const val& input){return transform(input,true);}
 val decodeResult(const val& input){return transform(input,false);}
 val status()const{auto result=val::object();result.set("failed",cipher.is_failed());result.set("sent",cipher.sent_serial());result.set("received",cipher.received_serial());return result;}
private:
 val transform(const val& input,bool encode) {
  auto result=val::object();
  try {auto bytes=inputBytes(input);result.set("bytes",copyBytes(encode?cipher.encode(std::move(bytes)):cipher.decode(std::move(bytes))));result.set("ok",true);}
  catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}return result;
 }
};
class BrowserFrameReader {
 mu::core::FrameReader reader;
public:
 val feed(const val& input) {
  if (!input.instanceof(val::global("Uint8Array"))) throw std::runtime_error("Expected Uint8Array");
  const auto size=input["byteLength"].as<unsigned>();
  if (size>mu::core::FrameReader::max_buffer) {
   // Preserve the core's terminal failure semantics for oversized input.
   reader.feed(mu::core::FrameReader::Bytes(mu::core::FrameReader::max_buffer+1));
  }
  mu::core::FrameReader::Bytes bytes(size);
  if(size) val(emscripten::typed_memory_view(size, bytes.data())).call<void>("set",input);
  auto frames=reader.feed(bytes);
  auto result=val::array();
  for (const auto& frame:frames) {
   // JS owns a copy, not a view into a destroyed C++ vector/WASM allocation.
   auto copy=val::global("Uint8Array").new_(val(emscripten::typed_memory_view(frame.size(),frame.data())));
   result.call<void>("push",copy);
  }
  return result;
 }
 val feedResult(const val& input) {
  auto result=val::object();
  try {result.set("frames",feed(input));result.set("ok",true);}
  catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}
  return result;
 }
 val finishResult() {
  auto result=val::object();
  try {reader.finish();result.set("ok",true);}
  catch(const std::exception& e){result.set("ok",false);result.set("error",std::string(e.what()));}
  return result;
 }
 void finish(){reader.finish();}
 unsigned pendingSize()const{return reader.pending_size();}
 bool failed()const{return reader.failed();}
};
}
EMSCRIPTEN_BINDINGS(mu_frame_reader) {
 emscripten::class_<ActorAnimation>("ActorAnimation").constructor<const val&>().constructor<const val&,unsigned>().function("configureMonster",&ActorAnimation::configureMonster).function("configureCharacter",&ActorAnimation::configureCharacter).function("configureMount",&ActorAnimation::configureMount).function("actionFrames",&ActorAnimation::actionFrames).function("sampleActionOnceResult",&ActorAnimation::sampleActionOnceResult).function("sampleActionResult",&ActorAnimation::sampleActionResult).function("addPart",&ActorAnimation::addPart).function("sampleResult",&ActorAnimation::sampleResult);
 emscripten::class_<ModelAnimation>("ModelAnimation").constructor<const val&>().function("sampleResult",&ModelAnimation::sampleResult).function("sampleBonesResult",&ModelAnimation::sampleBonesResult);
 emscripten::class_<BodyAnimation>("BodyAnimation").constructor<const val&,unsigned>().function("configureHelper",&BodyAnimation::configureHelper).function("addPart",&BodyAnimation::addPart).function("setWeapon",&BodyAnimation::setWeapon).function("addWeapon",&BodyAnimation::addWeapon).function("sampleResult",&BodyAnimation::sampleResult).function("setSafeZone",&BodyAnimation::setSafeZone).function("configureWorld",&BodyAnimation::configureWorld).function("configureMountedWorld",&BodyAnimation::configureMountedWorld).function("configureMotion",&BodyAnimation::configureMotion).function("configureCombat",&BodyAnimation::configureCombat).function("configureSpell",&BodyAnimation::configureSpell).function("sampleMotionResult",&BodyAnimation::sampleMotionResult);

 emscripten::function("boundGeometryResult",&boundGeometryResult);
 emscripten::function("inventoryArmorGeometryResult",&inventoryArmorGeometryResult);
 emscripten::function("textureResult",&textureResult);
 emscripten::function("bmdGeometryResult",&bmdGeometryResult);
 emscripten::function("bmdModelResult",&bmdModelResult);
 emscripten::function("boneNamesResult",&boneNamesResult);
 emscripten::function("attachmentGeometryResult",&attachmentGeometryResult);
 emscripten::function("equipmentActionsResult",&equipmentActionsResult);
 emscripten::function("equipmentGripResult",&equipmentGripResult);
 emscripten::function("armorResourceResult",&armorResourceResult);
 emscripten::function("itemResourceResult",&itemResourceResult);
 emscripten::function("companionsResult",&companionsResult);
 emscripten::function("appearanceResult",&appearanceResult);
 emscripten::function("characterCodecResult",&characterCodecResult);
 emscripten::function("loginPacketResult",&loginPacketResult);
 emscripten::function("loginResponseResult",&loginResponseResult);
 emscripten::function("validateKeyFileResult",&validateKeyFileResult);
 emscripten::class_<BrowserPacketCipher>("PacketCipher")
 .constructor<val,val>()
 .function("encodeResult",&BrowserPacketCipher::encodeResult)
 .function("decodeResult",&BrowserPacketCipher::decodeResult)
 .function("status",&BrowserPacketCipher::status);
 emscripten::function("identityPacketResult",&identityPacketResult);
 emscripten::function("packetFilterResult",&packetFilterResult);
 emscripten::function("deriveProfileResult",&deriveProfileResult);
 emscripten::function("gameGreetingResult",&gameGreetingResult);
 emscripten::function("transformTransportResult",&transformResult);
 emscripten::function("connectCodecResult",&connectCodecResult);
 emscripten::class_<BrowserFrameReader>("FrameReader")
 .constructor<>()
 .function("feed",&BrowserFrameReader::feed)
 .function("finish",&BrowserFrameReader::finish)
 .function("feedResult",&BrowserFrameReader::feedResult)
 .function("finishResult",&BrowserFrameReader::finishResult)
 .function("pendingSize",&BrowserFrameReader::pendingSize)
 .function("failed",&BrowserFrameReader::failed);
}
