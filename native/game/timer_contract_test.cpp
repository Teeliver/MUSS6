#include "queue_timer.hpp"
namespace { int count=0,timerIndex=-1;void tick(void* context,BOOLEAN fired){assert(fired);++count;timerIndex=static_cast<int>(reinterpret_cast<std::intptr_t>(context));} }
int main(){
 CQueueTimer timer;const auto now=std::chrono::steady_clock::now();timer.CreateTimer(42,100,tick);
 timer.NativePoll(now);assert(count==0);
 timer.NativePoll(now+std::chrono::milliseconds(1200));assert(count==1&&timerIndex==42);
 timer.NativePoll(now+std::chrono::seconds(20));assert(count==2); // A late tick causes no burst of hundreds of callbacks.
 timer.DeleteTimer(42);timer.NativePoll(now+std::chrono::seconds(21));assert(count==2);
 bool rejected=false;try{timer.CreateTimer(0,0,tick);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
}
