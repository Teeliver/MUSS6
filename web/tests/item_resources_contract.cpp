#include "mu/item_resources.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_RESOURCE_REFERENCE
#include "item_resource_reference.hpp"
#endif
int main(int argc,char**){
 for(unsigned index=0;index<8192;index++)for(unsigned level:{0u,1u,4u,15u}){
  auto path=mu::core::builtin_item_model(index,level);
#ifdef MU_RESOURCE_REFERENCE
  assert(path==mu::viewer::item_model_path({},mu::client::Item{index,level}).generic_string().substr(0));
#endif
  if(argc>1&&!path.empty())std::cout<<index<<'\t'<<level<<'\t'<<path<<'\n';
 }
 if(argc==1)std::cout<<"item resource contract passed\n";
}
