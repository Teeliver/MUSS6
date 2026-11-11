#include "queue_timer.hpp"
CQueueTimer gQueueTimer;
void CQueueTimer::CreateTimer(int index,int delay,WAITORTIMERCALLBACK callback){
 if(delay<=0||!callback)throw std::invalid_argument("Invalid game timer");
 timers_.emplace(index,Timer{std::chrono::steady_clock::now()+std::chrono::seconds(1),std::chrono::milliseconds(delay),callback});
}
void CQueueTimer::DeleteTimer(int index){timers_.erase(index);}
void CQueueTimer::NativePoll(std::chrono::steady_clock::time_point now){
 std::vector<std::pair<int,WAITORTIMERCALLBACK>> ready;
 for(auto& [index,timer]:timers_)if(now>=timer.due){
  // Avoid a callback burst after a slow database/script operation; retain the cadence.
  timer.due+=timer.period*((now-timer.due)/timer.period+1);ready.emplace_back(index,timer.callback);
 }
 for(const auto& [index,callback]:ready)if(timers_.count(index))callback(reinterpret_cast<void*>(static_cast<std::intptr_t>(index)),TRUE);
}
