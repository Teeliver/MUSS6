#include "mu/equipment_actions.hpp"
#include <cassert>
#include <iostream>
#ifdef MU_ACTIONS_REFERENCE
#include "actions_reference.hpp"
#endif
int main(){unsigned count=0;for(unsigned cls=0;cls<7;cls++)for(int right=-1;right<8192;right++)for(int left:{-1,0,2048,2055,2056})for(bool two:{false,true}){
 auto actual=mu::core::equipped_actions(cls,right,left,two);
#ifdef MU_ACTIONS_REFERENCE
 mu::viewer::CharacterResources r;r.idle=cls==2?2:cls==5?3:1;r.walk=(cls==2||cls==5)?16:15;r.run=25;
 if(right>=0)r.attachments[0]=mu::viewer::Item{unsigned(right)};if(left>=0)r.attachments[1]=mu::viewer::Item{unsigned(left)};
 mu::ui::ItemCatalog catalog;catalog.two=two;assert(actual==mu::viewer::equipped_actions(r,catalog));assert(mu::core::equipped_attack(right,left,two)==mu::viewer::attack_reference(r.attachments[0],r.attachments[1],catalog));
#endif
 for(auto a:actual)assert(a<512);count++;
 }
 assert((mu::core::equipped_actions(0,0,-1,false)==std::array<unsigned,3>{4,17,26}));
 assert((mu::core::equipped_actions(2,-1,2048,false)==std::array<unsigned,3>{8,21,30}));
 bool failed=false;try{mu::core::item_two_hand({},0);}catch(const std::runtime_error&){failed=true;}assert(failed);
 std::cout<<"equipment actions contract passed: "<<count<<" cases\n";
}
