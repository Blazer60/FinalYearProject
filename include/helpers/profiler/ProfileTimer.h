/**
 * @file Timer.h
 * @author Ryan Purse
 * @date 07/10/2023
 */


#pragma once

#include "Pch.h"


namespace debug
{
    /**
     * @author Ryan Purse
     * @date 07/10/2023
     */
    class ProfileTimer
    {
    public:
        explicit ProfileTimer(std::string_view name);
        ~ProfileTimer();
        
        void stop();
    
    protected:
        std::string_view mName;
        std::chrono::time_point<std::chrono::steady_clock> mStartPoint;
        bool mStopped { false };
        uint64_t mId { 0 };
    };
}


#define CONCAT(a, b) a ## b
#ifdef ENABLE_PROFILING
    #define PROFILE_FUNC() debug::ProfileTimer CONCAT(debugProfileTimer, __LINE__)(__FUNCTION__)
    #define PROFILE_SCOPE_BEGIN(id, name) debug::ProfileTimer id(name)
    #define PROFILE_SCOPE_END(name) name.stop();
#else
    #define PROFILE_FUNC()
    #define PROFILE_SCOPE_BEGIN(id, name)
    #define PROFILE_SCOPE_END(name)
#endif
