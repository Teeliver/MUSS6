#pragma once
#include <array>
#include <cmath>
#include <stdexcept>
namespace mu::core {
using GripMatrix=std::array<float,16>; // Column-major, native simd convention.
inline GripMatrix grip_identity(){return {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};}
inline GripMatrix grip_multiply(const GripMatrix& a,const GripMatrix& b){GripMatrix out{};for(unsigned c=0;c<4;c++)for(unsigned r=0;r<4;r++)for(unsigned k=0;k<4;k++)out[c*4+r]+=a[k*4+r]*b[c*4+k];return out;}
inline GripMatrix grip_euler(std::array<float,3> a,std::array<float,3> p){
 constexpr float radians=3.14159265358979323846f/180.f;for(auto& v:a)v*=radians;
 const auto cx=std::cos(a[0]),sx=std::sin(a[0]),cy=std::cos(a[1]),sy=std::sin(a[1]),cz=std::cos(a[2]),sz=std::sin(a[2]);
 return {cz*cy,sz*cy,-sy,0,cz*sy*sx-sz*cx,sz*sy*sx+cz*cx,cy*sx,0,cz*sy*cx+sz*sx,sz*sy*cx-cz*sx,cy*cx,0,p[0],p[1],p[2],1};
}
// Ported policy from native/client/equipment_grip.hpp. Not item equip eligibility.
inline GripMatrix equipment_grip_matrix(unsigned index,bool right){
 unsigned group=index/512,type=index%512;std::array<float,3> angles={70,0,90},p={-20,5,40};
 if(group==4){if((type>=8&&type<15)||type==16||type==18||type==19){angles={0,20,180};p={-10,8,40};}else{p={-10,5,10};if(type==20||type==23){angles={-60,0,-80};p={-5,20,type==23?-5.f:0.f};}}}
 if(group==5&&type==9){angles={110,180,90};p={-10,5,-10};}else if(group==5&&type==8)p={-10,5,10};
 if(group==6){p={-10,0,0};if(type==6){angles={30,0,90};p={-15,0,-25};}if(type==14||type==15){angles={50,0,90};p={-28,0,-25};}if(type==16){angles={30,0,90};p={-20,0,-20};}}
 auto m=grip_euler(angles,p);if(!right&&group!=6&&group!=4)m=grip_multiply(m,grip_euler({145,0,275},{0,10,-30}));return m;
}
struct EquipmentGrip{unsigned bone;GripMatrix transform;};
inline EquipmentGrip equipment_grip(unsigned slot,unsigned index,bool safe){
 if((slot!=0&&slot!=1&&slot!=7&&!(slot==8&&index==13*512+1))||index>=8192)throw std::runtime_error("Invalid equipment attachment identity");
 if(slot==8)return {34u,grip_euler({0,0,0},{20,0,0})};
 if(slot==7){const bool robe=index==12*512+40,linkedWing=index==12*512+50;auto m=linkedWing?grip_euler({0,90,0},{10,-15,0}):robe?grip_euler({0,90,0},{-47,-7,0}):grip_euler({0,0,0},{0,0,15});return {robe||linkedWing?19u:47u,m};}
 const bool back=safe||index==4*512+7||index==4*512+15;
 return {back?47u:slot==0?33u:42u,back?equipment_grip_matrix(index,slot==0):grip_identity()};
}
}
