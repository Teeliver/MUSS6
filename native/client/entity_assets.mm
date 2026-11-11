#include "entity_assets.hpp"
#include "asset_images.hpp"
#include <algorithm>
#include <stdexcept>
namespace mu::viewer {namespace {void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}}
ActorCatalog load_actor_catalog(const std::filesystem::path& data){
 struct Spec{unsigned type;const char* name;bool npc;float scale,opacity,altitude;double speed,walk;std::vector<std::string> files;};
 const std::vector<Spec> specs={
 {226,"Pet Trainer",true,1,1,0,2.5,.25,{"Npc/Breeder.bmd"}},
 {235,"Priest Sevina",true,1,1,0,2.5,.25,{"Npc/Sevina01.bmd"}},
 {237,"Charon",true,1,1,0,2.5,.25,{"Npc/DevilNpc01.bmd"}},
 {238,"Chaos Goblin",true,1,1,0,2.5,.25,{"Npc/MixNpc01.bmd"}},
 {239,"Arena Guard",true,1,1,0,2.5,.25,{"Npc/Tournament01.bmd"}},
 {240,"Warehouse Keeper",true,1,1,0,2.5,.25,{"Npc/Storage01.bmd"}},
 {241,"Guild Master",true,1,1,0,2.5,.25,{"Npc/Master01.bmd"}},
 {242,"Elf Wizard",true,1,1,1.4,2.5,.25,{"Npc/ElfWizard01.bmd"}},
 {243,"Craftsman",true,1,1,0,2.5,.25,{"Npc/ElfMerchant01.bmd"}},
 {244,"Devias Merchant",true,1,1,0,2.5,.25,{"Npc/SnowMerchant01.bmd"}},
 {245,"Devias Wizard",true,1,1,0,2.5,.25,{"Npc/SnowWizard01.bmd"}},
 {246,"Devias Smith",true,1,1,0,2.5,.25,{"Npc/SnowSmith01.bmd"}},
 {248,"Wandering Merchant",true,1,1,0,2.5,.25,{"Npc/Man01.bmd","Npc/ManHead02.bmd","Npc/ManUpper02.bmd","Npc/ManGloves02.bmd","Npc/ManBoots02.bmd"}},
 {250,"Wandering Merchant",true,1,1,0,2.5,.25,{"Npc/Man01.bmd","Npc/ManHead01.bmd","Npc/ManUpper01.bmd","Npc/ManGloves01.bmd","Npc/ManBoots01.bmd"}},
 {251,"Blacksmith Hans",true,.95,1,0,2.5,.25,{"Npc/Smith01.bmd"}},
 {253,"Amy",true,1,1,0,2.5,.25,{"Npc/Girl01.bmd","Npc/GirlHead01.bmd","Npc/GirlUpper01.bmd","Npc/GirlLower01.bmd"}},
 {254,"Wizard Pasi",true,1,1,0,2.5,.25,{"Npc/Wizard01.bmd"}},
 {255,"Lumen",true,1,1,0,2.5,.25,{"Npc/Female01.bmd","Npc/FemaleHead02.bmd","Npc/FemaleUpper02.bmd","Npc/FemaleLower02.bmd","Npc/FemaleBoots02.bmd"}},
 {543,"Gens Duprian",true,1,1,0,2.5,.25,{"Npc/duprian.bmd"}},
 {544,"Gens Vanert",true,1,1,0,2.5,.25,{"Npc/barnert.bmd"}},
 {2,"Budge Dragon",false,.5,1,0,2.5,.7,{"Monster/Monster03.bmd"}},
 {3,"Spider",false,.4,1,0,2.5,1.2,{"Monster/Monster10.bmd"}},
 {11,"Ghost",false,1,.4,0,3.75,.34,{"Monster/Monster08.bmd"}},
 {12,"Larva",false,.6,1,0,2.5,.6,{"Monster/Monster07.bmd"}}
 };
 ActorCatalog catalog;std::map<std::string,std::shared_ptr<const mu::assets::Image>> images;
 for(const auto& spec:specs){auto visual=std::make_shared<ActorVisual>();visual->type=spec.type;visual->npc=spec.npc;visual->name=spec.name;visual->scale=spec.scale;visual->opacity=spec.opacity;visual->altitude=spec.altitude;visual->moveSpeed=spec.speed;visual->walkKeySpeed=spec.walk;
  auto skeleton=mu::assets::load_model(data/spec.files.front(),{0});if(!spec.npc&&skeleton.actionCount>6)skeleton=mu::assets::load_model(data/spec.files.front(),{0,2,3,4,6});else if(!spec.npc&&skeleton.actionCount>2)skeleton=mu::assets::load_model(data/spec.files.front(),{0,2});mu::assets::posed_vertices(mu::assets::Mesh{},skeleton.bones);visual->bones=skeleton.bones;auto freeze=[&](unsigned action){if(!skeleton.clips.count(action)||!skeleton.lockedPositions.at(action)||skeleton.bones.empty())return;auto& frames=skeleton.clips.at(action);auto first=frames.front()[0].position;for(auto& frame:frames){frame[0].position.x=first.x;frame[0].position.y=first.y;}};freeze(0);freeze(2);freeze(6);for(unsigned i=0;i<2;i++){unsigned action=3+i;if(skeleton.clips.count(action)){freeze(action);visual->attack[i]=skeleton.clips.at(action);visual->attackPeriod[i]=std::max(1u,unsigned(visual->attack[i].size())-(skeleton.lockedPositions.at(action)?1:0));}}if(skeleton.clips.count(6)){visual->death=skeleton.clips.at(6);visual->deathPeriod=std::max(1u,unsigned(visual->death.size())-(skeleton.lockedPositions.at(6)?1:0));}visual->idle=skeleton.clips.at(0);visual->idlePeriod=std::max(1u,unsigned(visual->idle.size())-(skeleton.lockedPositions.at(0)?1:0));if(skeleton.clips.count(2)){visual->walk=skeleton.clips.at(2);visual->walkPeriod=std::max(1u,unsigned(visual->walk.size())-(skeleton.lockedPositions.at(2)?1:0));}need(visual->idlePeriod>0,"NPC idle cycle has no usable frame");
  for(unsigned partIndex=0;partIndex<spec.files.size();partIndex++){auto path=data/spec.files[partIndex];auto model=partIndex?mu::assets::load_model(path):skeleton;
   for(auto& mesh:model.meshes){for(const auto& vertex:mesh.vertices){need(vertex.bone>=0&&unsigned(vertex.bone)<visual->bones.size()&&unsigned(vertex.bone)<model.bones.size(),"NPC vertex uses an unsupported skeleton binding");auto i=unsigned(vertex.bone);need(model.bones[i].name==visual->bones[i].name&&model.bones[i].parent==visual->bones[i].parent,"NPC part skeleton differs from primary model");}
    auto texturePath=resolve_model_texture(path.parent_path(),mesh.texture);auto key=texturePath.string();auto skin=images.find(key);if(skin==images.end())skin=images.emplace(key,std::make_shared<mu::assets::Image>(decode_texture(texturePath))).first;bool additive=(spec.type==238||spec.type==242)&&mesh.textureIndex==1;visual->parts.push_back({std::move(mesh),skin->second,additive});
   }
  }need(!visual->parts.empty(),"NPC visual has no meshes");catalog.emplace(spec.type,std::move(visual));
 }return catalog;
}

}
