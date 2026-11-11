#include "mu/companions.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_COMPANIONS_REFERENCE
#include "companions_reference.hpp"
#endif
using namespace mu::core;
int main(){
#ifdef MU_COMPANIONS_REFERENCE
 const int wings[]={511,0,1,2,3,4,5,6,30,36,37,38,39,40,41,42,43,49,50,130,131,132,133,134,135,262,263,264,265,266,267,500};
 const int pets[]={511,0,1,2,3,4,37,64,65,67,80,106,123};
 for(int wing:wings)for(int pet:pets)for(unsigned variant=0;variant<8;variant++){
  int ids[9]={};ids[7]=wing;ids[8]=pet;companions_reference::Input input;input.Inventory[42]=variant;
  auto packed=companions_reference::pack(ids,&input);auto a=decode_companions(WireBytes(packed.CharSet,packed.CharSet+18));
  assert(a.wing.status==(wing==511?0u:wing==500?2u:(wing>=130&&wing<=135)?4u:1u));
  if(wing!=511&&wing!=500)assert(a.wing.index==unsigned(12*512+wing));
  if(wing==500)assert(a.wing.customIndex==3);
  assert(a.pet.status==(pet==511?0u:1u));if(pet!=511)assert(a.pet.index==unsigned(13*512+pet));
  if(pet==37)assert(a.pet.variant==((variant&1)?1u:(variant&2)?2u:(variant&4)?4u:0u));
 }
#endif
 WireBytes b(18);b[5]=12;auto a=decode_companions(b);assert(a.wing.status==3);b[16]=192;a=decode_companions(b);assert(a.pet.status==3);
 bool failed=false;try{decode_companions(WireBytes(19));}catch(...){failed=true;}assert(failed);
 std::cout<<"companions contract passed\n";
}
