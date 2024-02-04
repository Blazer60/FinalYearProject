/**
 * @file Timer.cpp
 * @author Ryan Purse
 * @date 07/10/2023
 */


#include "ProfileTimer.h"
#include "Profiler.h"

#include <utility>
#include <thread>

namespace debug
{
    ProfileTimer::ProfileTimer(std::string_view name)
        : mName(name), mStartPoint(std::chrono::high_resolution_clock::now()), mId(profiler->getNewId())
    {
    }
    
    ProfileTimer::~ProfileTimer()
    {
        if (!mStopped)
            stop();
    }
    
    void ProfileTimer::stop()
    {
        const auto endTimePoint = std::chrono::high_resolution_clock::now();
        
        const long long start = std::chrono::time_point_cast<std::chrono::nanoseconds>(mStartPoint).time_since_epoch().count();
        const long long end = std::chrono::time_point_cast<std::chrono::nanoseconds>(endTimePoint).time_since_epoch().count();
        
        mStopped = true;

        const uint32_t threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
        profiler->addResult({ mId, mName, start, end, threadId });
    }
}
