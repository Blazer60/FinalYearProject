/**
 * @file Timer.h
 * @author Ryan Purse
 * @date 07/10/2023
 */


#pragma once

#include "Pch.h"


namespace engine
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
    };
}


#define CONCAT(a, b) a ## b
#define PROFILE_FUNC() engine::ProfileTimer CONCAT(engineProfileTimer, __LINE__)(__FUNCTION__)
#define PROFILE_SCOPE_BEGIN(id, name) engine::ProfileTimer id(name)
#define PROFILE_SCOPE_END(name) name.stop();
