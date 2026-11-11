#pragma once
#include "mu/platform.hpp"
class CQueueTimer{
public:
 void CreateTimer(int index,int delay,WAITORTIMERCALLBACK callback);
 void DeleteTimer(int index);
 void NativePoll(std::chrono::steady_clock::time_point now=std::chrono::steady_clock::now());
 void NativeClear(){timers_.clear();}
private:
 struct Timer{std::chrono::steady_clock::time_point due;std::chrono::milliseconds period;WAITORTIMERCALLBACK callback;};
 std::map<int,Timer> timers_;
};
extern CQueueTimer gQueueTimer;
