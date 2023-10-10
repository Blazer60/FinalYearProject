/**
 * @file ProfileResults.cpp
 * @author Ryan Purse
 * @date 07/10/2023
 */


#include "Profiler.h"
#include "Timers.h"

Profiler *profiler;

namespace debug
{
    bool ProfileNode::tryInsert(ProfileResult result)
    {
        if (result.startMicroSeconds >= startNanoSeconds && result.stopMicroSeconds <= stopNanoSeconds)
        {
            bool isInserted = false;
            for (ProfileNode &child : children)
                isInserted |= child.tryInsert(result);
            
            if (!isInserted)
                children.push_back({ result.id, result.name, result.startMicroSeconds, result.stopMicroSeconds, { } });
            
            return true;
        }
        return false;
    }
}

void Profiler::addResult(const debug::ProfileResult &result)
{
    mResults.push_back(result);
}

void Profiler::createTree()
{
    std::sort(mResults.begin(), mResults.end(), [](const debug::ProfileResult &lhs, const debug::ProfileResult &rhs) {
        return lhs.id < rhs.id;
    });
    
    for (const auto &item : mResults)
    {
        bool isInserted = false;
        for (auto &node : mTree)
            isInserted |= node.tryInsert(item);
        
        if (!isInserted)
            mTree.push_back({ item.id, item.name, item.startMicroSeconds, item.stopMicroSeconds, {} });
    }
}

void Profiler::updateAndClear()
{
#ifdef ENABLE_PROFILING
    if (!mIsFrozen)
    {
        mTimer += timers::deltaTime<float>();
        if (mTimer > mUpdateRate)
        {
            mTimer -= mUpdateRate;
            mTree.clear();
            createTree();
        }
    }
#else
    if (!mResults.empty())
            LOG_MINOR("Profiling was accessed in a non-profiling build");
#endif  // ENABLE_PROFILING
    
    mResults.clear();
    mId = 0;
}

bool Profiler::isFrozen() const
{
    return mIsFrozen;
}

void Profiler::setFreeze(bool isFrozen)
{
    mIsFrozen = isFrozen;
}

void Profiler::setUpdateRate(float updateRate)
{
    mUpdateRate = updateRate;
}

const std::vector<debug::ProfileNode> &Profiler::getTree() const
{
    return mTree;
}

uint64_t Profiler::getNewId()
{
    return ++mId;
}
