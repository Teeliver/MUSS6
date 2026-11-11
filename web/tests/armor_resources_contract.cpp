#include "mu/armor_resources.hpp"
#include <cassert>
#include <array>
#include <iostream>
#ifdef MU_ARMOR_REFERENCE
#include "armor_reference.hpp"
#endif
int main(){
 unsigned mapped=0,unmapped=0;
 for(unsigned c=0;c<7;c++)for(unsigned t=0;t<3;t++)for(unsigned p=0;p<5;p++)for(unsigned type=0;type<512;type++){
  std::string path;bool valid=true;try{path=mu::core::armor_model(p,type,c,t);}catch(const std::runtime_error&){valid=false;}
#ifdef MU_ARMOR_REFERENCE
  std::string ref;bool refValid=true;try{ref=mu::viewer::armor_model(p,type,c,t);}catch(const std::runtime_error&){refValid=false;}
  assert(valid==refValid);if(valid)assert(path==ref);
#endif
  if(valid){assert(!path.empty());mapped++;}else unmapped++;
 }
 for(const auto& input: {std::array<unsigned,4>{5,0,0,0},{0,0,7,0},{0,0,0,3}}){bool failed=false;try{mu::core::armor_model(input[0],input[1],input[2],input[3]);}catch(const std::runtime_error&){failed=true;}assert(failed);}
 assert(mapped>0&&unmapped>0);std::cout<<"armor resource contract passed: "<<mapped<<" mapped, "<<unmapped<<" unmapped\n";
}
