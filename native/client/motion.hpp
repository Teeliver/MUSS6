#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
namespace mu::viewer {
struct MotionPoint { double x=0,y=0,z=0,yaw=0; };
// MovePath reference four-point curve, sampled by distance rather than by
// parameter time. CharacterMoveSpeed: 12/15 units per 25 Hz tick, tile=100.
class MotionTrack {
 std::deque<MotionPoint> path;MotionPoint from,current,previous,segmentNext;double traveled=0,quiet=1,speed=3,movementRate=1,yawTarget=0;
 std::array<double,65> arc{};bool cached=false;
 MotionPoint curve(double t)const{auto target=path.front(),next=segmentNext;auto c=[&](double a,double b,double c,double d){return .5*((2*b)+(-a+c)*t+(2*a-5*b+4*c-d)*t*t+(-a+3*b-3*c+d)*t*t*t);};return {c(previous.x,from.x,target.x,next.x),from.y+(target.y-from.y)*t,c(previous.z,from.z,target.z,next.z),target.yaw};}
 void buildArc(){segmentNext=path.size()>1?path[1]:path.front();arc[0]=0;auto a=curve(0);for(unsigned i=1;i<arc.size();i++){auto b=curve(double(i)/(arc.size()-1));arc[i]=arc[i-1]+std::hypot(b.x-a.x,b.z-a.z);a=b;}cached=true;}
public:
 explicit MotionTrack(MotionPoint initial={}):from(initial),current(initial),previous(initial),yawTarget(initial.yaw){}
 bool pending()const{return !path.empty();}
 double remainingDistance()const{if(path.empty())return 0;double distance=std::hypot(path.front().x-current.x,path.front().z-current.z);for(size_t i=1;i<path.size();i++)distance+=std::hypot(path[i].x-path[i-1].x,path[i].z-path[i-1].z);return distance;}

 void clearPath(){path.clear();from=previous=current;traveled=0;cached=false;}
 void face(double yaw){yawTarget=yaw;}
 void setSpeed(double tilesPerSecond){speed=std::clamp(tilesPerSecond,.1,20.);}
 void setMovementRate(double rate){rate=std::clamp(rate,.5,1.25);speed=speed/movementRate*rate;movementRate=rate;}
 void setRunning(bool run){speed=(run?3.75:3.)*movementRate;}
 void enqueue(MotionPoint target){auto anchor=path.empty()?current:path.back();double distance=std::hypot(target.x-anchor.x,target.z-anchor.z);
  if(distance<1e-6)return;
  if(distance>2.2||path.size()>=16){path.clear();current=from=previous=target;face(target.yaw);traveled=0;cached=false;quiet=1;return;}
  if(path.empty()){from=current;traveled=0;cached=false;}path.push_back(target);quiet=0;
 }
 MotionPoint advance(double dt){dt=std::clamp(dt,0.,.1);double time=dt,facing=current.yaw;
  while(time>0&&!path.empty()){if(!cached)buildArc();double length=arc.back(),remaining=std::max(0.,length-traveled);double used=std::min(time,remaining/speed);traveled+=used*speed;time-=used;
   auto upper=std::lower_bound(arc.begin()+1,arc.end(),traveled);unsigned hi=std::min<unsigned>(upper-arc.begin(),arc.size()-1);double fraction=arc[hi]>arc[hi-1]?(traveled-arc[hi-1])/(arc[hi]-arc[hi-1]):0;double t=(hi-1+fraction)/(arc.size()-1);current=curve(t);
   auto ahead=curve(std::min(1.,t+.001)),behind=curve(std::max(0.,t-.001));double vx=ahead.x-behind.x,vz=ahead.z-behind.z;if(std::hypot(vx,vz)>1e-7)yawTarget=std::atan2(vx,vz);
   if(traveled>=length-1e-9){current=path.front();previous=from;from=current;path.pop_front();traveled=0;cached=false;}
  }
  if(path.empty())quiet+=time;double turn=std::remainder(yawTarget-facing,2*3.141592653589793);current.yaw=facing+turn*(1-std::pow(.5,dt*25));return current;
 }
 bool moving()const{return !path.empty()||quiet<.12;}
 MotionPoint position()const{return current;}
};
// SetPlayerWalk reference: 40 ticks outside safe zone with eligible DW boots.
class RunPolicy { double ticks=0;public:
 bool update(double dt,bool moving,bool safeZone,bool eligible){if(!moving||safeZone||!eligible){ticks=0;return false;}ticks=std::min(40.,ticks+std::clamp(dt,0.,.1)*25);return ticks>=40.-1e-6;}
};
}
