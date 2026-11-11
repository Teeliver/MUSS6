#pragma once
#include "animation_pose.hpp"
namespace mu::core::bmd {
inline void normalize_motion_roots(Model& model,const std::vector<unsigned>& actions,unsigned reference){
 const auto& base=model.clips.at(reference).front();for(auto action:actions)for(auto& pose:model.clips.at(action)){need(pose.size()==base.size(),"Motion root bone count mismatch");for(unsigned i=0;i<pose.size();i++)if(pose[i].parent<0){pose[i].position.x=base[i].position.x;pose[i].position.y=base[i].position.y;}}
}
// Native world_view.mm idle period/1.2, walk .33*25, run .34*25, blend .12s.
class MotionAnimation {
 const Model& model;std::vector<unsigned> actions;std::vector<Bone> current,before;unsigned mode=0;double last=0,phase=0,blendAge=.12;bool started=false,worldModes=false;double sampledFrame=0;
 public:
 MotionAnimation(const Model& source,std::vector<unsigned> selected,bool world=false):model(source),actions(selected),worldModes(world){for(auto action:actions)need(model.clips.count(action)!=0,"Motion clip unavailable");}
 double frame()const{return sampledFrame;}
 unsigned currentMode()const{return mode;}
 std::vector<Bone> sample(double seconds,unsigned requested){
 need(std::isfinite(seconds)&&seconds>=0&&requested<actions.size()&&(!started||seconds>=last),"Invalid motion timeline");
 const double dt=started?seconds-last:0;last=seconds;started=true;
 if(current.empty()){current=sample_pose(model,actions[0],0);before=current;}
 if(requested!=mode){const bool continuing=(mode==1||mode==2||(worldModes&&mode==5))&&(requested==1||requested==2||(worldModes&&requested==5));before=current;blendAge=0;if(!continuing)phase=0;mode=requested;}
 phase+=dt;blendAge+=dt;const unsigned action=actions[mode];unsigned period=unsigned(model.clips.at(action).size());if(model.lockedPositions.at(action)&&period>1)period--;
 const double rate=worldModes?(mode>=16?11.25:mode>=10?(mode>=14?5.:10.):mode==8||mode==9?6.25:mode==3||mode>=6?period/.6:(mode==0||mode==4)?period/1.2:mode==2?.34*25:.33*25):mode>=3?period/.6:mode==0?period/1.2:mode==2?.34*25:.33*25;
 const bool death=worldModes&&(mode==16||mode==17);
 const double clock=death?std::min(phase,double(period-1)/rate):phase;
 sampledFrame=death?clock*rate:std::fmod(clock*rate,double(std::max(1u,period)));auto target=sample_pose(model,action,clock,rate);current=blend_pose(before,target,std::min(1.,blendAge/.12));return current;
 }
};
}
