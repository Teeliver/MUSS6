#include "../../native/client/evil_spirit.hpp"
#include <iostream>
#include <iomanip>
int main(){std::cout<<std::setprecision(17);for(unsigned seed:{1u,42u,999u}){mu::client::EvilSpirits s(0,0,seed);for(unsigned tick=0;tick<50;tick++){s.step(0,0);for(unsigned i=0;i<4;i++){auto& h=s.heads[i];std::cout<<seed<<' '<<tick+1<<' '<<i<<' '<<h.p.x<<' '<<h.p.y<<' '<<h.p.z<<' '<<h.yaw<<' '<<h.pitch<<' '<<h.dyaw<<' '<<h.dpitch<<'\n';}}}}
