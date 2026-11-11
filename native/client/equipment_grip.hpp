#pragma once
#include <simd/simd.h>
#include <cmath>
namespace mu::viewer {
inline simd_float4x4 equipment_grip_matrix(unsigned index,bool right){unsigned group=index/512,type=index%512;simd_float3 angles={70,0,90},p={-20,5,40};if(group==4){if((type>=8&&type<15)||type==16||type==18||type==19){angles={0,20,180};p={-10,8,40};}else {p={-10,5,10};if(type==20||type==23){angles={-60,0,-80};p={-5,20,type==23?-5.f:0.f};}}}if(group==5&&type==9){angles={110,180,90};p={-10,5,-10};}else if(group==5&&type==8)p={-10,5,10};if(group==6){p={-10,0,0};if(type==6){angles={30,0,90};p={-15,0,-25};}if(type==14||type==15){angles={50,0,90};p={-28,0,-25};}if(type==16){angles={30,0,90};p={-20,0,-20};}}
 auto matrix=[](simd_float3 a,simd_float3 p){a*=float(M_PI/180);auto q=simd_mul(simd_quaternion(a.z,simd_make_float3(0,0,1)),simd_mul(simd_quaternion(a.y,simd_make_float3(0,1,0)),simd_quaternion(a.x,simd_make_float3(1,0,0))));auto m=simd_matrix4x4(q);m.columns[3]=simd_make_float4(p,1);return m;};auto m=matrix(angles,p);if(!right&&group!=6&&group!=4)m=simd_mul(m,matrix(simd_make_float3(145,0,275),simd_make_float3(0,10,-30)));return m;}
inline unsigned equipment_attachment_bone(unsigned slot,unsigned index,bool safe){if(slot==7)return index==12*512+40?19:47;bool back=safe||index==4*512+7||index==4*512+15;return back?47:slot==0?33:42;}
inline simd_float4x4 equipment_attachment_transform(unsigned slot,unsigned index,bool safe){if(slot==7){auto m=index==12*512+40?simd_matrix4x4(simd_quaternion(float(M_PI/2),simd_make_float3(0,1,0))):matrix_identity_float4x4;m.columns[3]=index==12*512+40?simd_make_float4(-47,-7,0,1):simd_make_float4(0,0,15,1);return m;}bool back=safe||index==4*512+7||index==4*512+15;return back?equipment_grip_matrix(index,slot==0):matrix_identity_float4x4;}
}
