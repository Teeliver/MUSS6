#include "mu/texture.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_TEXTURE_REFERENCE
#include "assets.hpp"
#endif
using namespace mu::core;
int main(){
 for(unsigned bits:{24u,32u})for(unsigned flags:{0u,16u,32u,48u}){
  WireBytes b(22);b[6]=2;b[16]=2;b[18]=2;b[20]=bits;b[21]=flags;
  for(unsigned p=0;p<4;p++){b.push_back(p+1);b.push_back(p+2);b.push_back(p+3);if(bits==32)b.push_back(100+p);}
  auto image=decode_ozt(b);assert(image.width==2&&image.height==2&&image.bytes.size()==16);
#ifdef MU_TEXTURE_REFERENCE
  auto original=mu::assets::decode_tga(b);assert(image.bytes==original.rgba);
#endif
  for(unsigned n=0;n<b.size();n++){bool rejected=false;try{decode_ozt(WireBytes(b.begin(),b.begin()+n));}catch(...){rejected=true;}assert(rejected);}
 }
 std::cout<<"texture contract passed\n";
}
