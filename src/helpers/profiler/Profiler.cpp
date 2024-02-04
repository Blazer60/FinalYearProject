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
        if (result.startNanoSeconds >= startNanoSeconds && result.stopNanoSeconds <= stopNanoSeconds)
        {
            bool isInserted = false;
            for (ProfileNode &child : children)
                isInserted |= child.tryInsert(result);
            
            if (!isInserted)
                children.push_back({ result.id, result.name, result.startNanoSeconds, result.stopNanoSeconds, { } });
            
            return true;
        }
        return false;
    }
}

Profiler::~Profiler()
{
    if (mIsRecordingSnapshot)
        endSnapshot();
}

void Profiler::addResult(const debug::ProfileResult &result)
{
    if (mIsRecordingSnapshot)
        mSnapshotResults.push_back(result);
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
            mTree.push_back({ item.id, item.name, item.startNanoSeconds, item.stopNanoSeconds, {} });
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

void Profiler::beginSnapshot(const std::string& filePath)
{
    mSnapshotFilePath = filePath;
    mIsRecordingSnapshot = true;
    mProfileCount = 0;
}

void Profiler::endSnapshot()
{
    mOutputSteam.open(mSnapshotFilePath);
    mOutputSteam << "{\"otherData\": {},\"traceEvents\":[";

    for (const auto & snapshot : mSnapshotResults)
        writeProfile(snapshot);

    mOutputSteam << "]}";
    mOutputSteam.flush();
    mOutputSteam.close();

    mSnapshotResults.clear();
    mIsRecordingSnapshot = false;
}

void Profiler::writeProfile(const debug::ProfileResult& result)
{
    if (mProfileCount++ > 0)
        mOutputSteam << ",";

    std::string name = std::string(result.name);
    std::replace(name.begin(), name.end(), '"', '\'');

    mOutputSteam << "{";
    mOutputSteam << "\"cat\":\"function\",";
    const auto time = static_cast<long long>((result.stopNanoSeconds - result.startNanoSeconds) * 0.001);
    mOutputSteam << "\"dur\":" << time << ",";
    mOutputSteam << "\"name\":\"" << name << "\",";
    mOutputSteam << "\"ph\":\"X\",";
    mOutputSteam << "\"pid\":0,";
    mOutputSteam << "\"tid\":" << result.threadId << ",";
    const auto startTime = static_cast<long long>(result.startNanoSeconds * 0.001);
    mOutputSteam << "\"ts\":" << startTime;
    mOutputSteam << "}";

    mOutputSteam.flush();
}

uint64_t Profiler::getNewId()
{
    return ++mId;
}
