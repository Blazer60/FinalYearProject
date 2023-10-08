/**
 * @file ProfileResults.h
 * @author Ryan Purse
 * @date 07/10/2023
 */


#pragma once

#include "Pch.h"


extern class Profiler *profiler;

namespace debug
{
    struct ProfileResult
    {
        std::string_view name;
        long long startMicroSeconds;
        long long stopMicroSeconds;
    };
    
    struct ProfileNode
    {
        std::string_view name;
        long long startNanoSeconds;
        long long stopNanoSeconds;
        std::vector<ProfileNode> children;
        
        bool tryInsert(ProfileResult result);
    };
}

/**
 * @author Ryan Purse
 * @date 07/10/2023
 */
class Profiler
{
public:
    void addResult(const debug::ProfileResult &result);
    void updateAndClear();
    
    bool isFrozen() const;
    void setFreeze(bool isFrozen);
    void setUpdateRate(float updateRate);
    const std::vector<debug::ProfileNode> &getTree() const;

protected:
    void createTree();

protected:
    std::vector<debug::ProfileResult> mResults;
    std::vector<debug::ProfileNode> mTree;
    float mUpdateRate { 0.1f };
    float mTimer { 0.f };
    bool mIsFrozen { false };
};

