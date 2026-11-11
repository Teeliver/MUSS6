#pragma once
#include <array>
#include <deque>
#include <cmath>
#include <algorithm>
#include <cstdint>
namespace mu::client {
// ZzzCharacter AT_SKILL_EVIL / ZzzEffectJoint BITMAP_JOINT_SPIRIT subtype0:
// four headings, velocity70, life49, tails6, homing10 degrees per25Hz tick.
struct SpiritPoint {double x=0,y=0,z=1;};
struct SpiritHead {SpiritPoint p;double yaw=0,pitch=0,dyaw=0,dpitch=0;std::deque<SpiritPoint> tail;};
struct EvilSpirits {
 std::array<SpiritHead,4> heads;unsigned frame=0;std::uint32_t random;double accumulator=0;
 EvilSpirits(double x,double y,unsigned seed):random(seed?seed:1){for(unsigned i=0;i<4;i++){heads[i].p={x,y,1};heads[i].yaw=i*90;heads[i].tail.push_back(heads[i].p);}}
 unsigned next(){random^=random<<13;random^=random>>17;random^=random<<5;return random;}
 static double turn(double a,double b,double limit){return a+std::clamp(std::remainder(b-a,360.),-limit,limit);}
 void step(double x,double y){if(frame>=50)return;constexpr double rad=3.141592653589793/180;for(auto& h:heads){double sy=std::sin(h.yaw*rad),cy=std::cos(h.yaw*rad),sp=std::sin(h.pitch*rad),cp=std::cos(h.pitch*rad);h.p.x+=.7*sy*cp;h.p.y-=.7*cy*cp;h.p.z-=.7*sp;double dx=x-h.p.x,dy=y-h.p.y,d=std::hypot(dx,dy);h.yaw=turn(h.yaw,std::atan2(dx,-dy)/rad,10);h.pitch=turn(h.pitch,-std::atan2(.8-h.p.z,d)/rad,10);h.dpitch+=(int(next()%32)-16)*.2;h.dyaw+=(int(next()%32)-16)*.8;h.pitch+=h.dpitch;h.yaw+=h.dyaw;h.dpitch*=.6;h.dyaw*=.8;if(h.p.z<1){h.dpitch=0;h.pitch=-5;}if(h.p.z>4){h.dpitch=0;h.pitch=5;}h.tail.push_front(h.p);if(h.tail.size()>6)h.tail.pop_back();}frame++;}
 void advance(double dt,double x,double y){accumulator+=dt;while(accumulator>=.04&&frame<50){accumulator-=.04;step(x,y);}}
 bool hitTick()const{return frame==5||frame==20||frame==35||frame==50;}
 bool touches(double x,double y)const{for(const auto& h:heads)if(std::hypot(h.p.x-x,h.p.y-y)<=1.5)return true;return false;}
};
}
