#include "mu/equipment_grip.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_GRIP_REFERENCE
#include "equipment_grip.hpp"
#endif
int main(){unsigned cases=0;for(unsigned slot:{0u,1u,7u})for(unsigned index=0;index<8192;index++)for(bool safe:{false,true}){
 auto actual=mu::core::equipment_grip(slot,index,safe);
#ifdef MU_GRIP_REFERENCE
 if(slot==7&&index==12*512+50){assert(actual.bone==19);auto expected=mu::core::grip_euler({0,90,0},{10,-15,0});for(unsigned i=0;i<16;i++)assert(std::abs(actual.transform[i]-expected[i])<.0001f);}else{
 assert(actual.bone==mu::viewer::equipment_attachment_bone(slot,index,safe));
 auto ref=mu::viewer::equipment_attachment_transform(slot,index,safe);
 for(unsigned c=0;c<4;c++)for(unsigned r=0;r<4;r++)assert(std::abs(actual.transform[c*4+r]-ref.columns[c][r])<0.0001f);
 }
#endif
 for(auto v:actual.transform)assert(std::isfinite(v));cases++;
 }
 assert(mu::core::equipment_grip(0,0,false).bone==33);assert(mu::core::equipment_grip(1,0,false).bone==42);
 assert(mu::core::equipment_grip(0,0,true).bone==47);assert(mu::core::equipment_grip(7,12*512+40,false).bone==19);
 for(unsigned slot:{2u,8u}){bool failed=false;try{mu::core::equipment_grip(slot,0,false);}catch(const std::runtime_error&){failed=true;}assert(failed);}
 std::cout<<"equipment grip contract passed: "<<cases<<" cases\n";
}
