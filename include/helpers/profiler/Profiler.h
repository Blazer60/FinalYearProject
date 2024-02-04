/**
 * @file Profiler.h
 * @author Ryan Purse
 * @date 07/10/2023
 */


#pragma once

#include <fstream>
#include "Pch.h"


extern class Profiler *profiler;

namespace debug
{
    struct ProfileResult
    {
        uint64_t id;
        std::string_view name;
        long long startNanoSeconds;
        long long stopNanoSeconds;
        uint32_t threadId;
    };
    
    struct ProfileNode
    {
        uint64_t id;
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
    ~Profiler();
    void addResult(const debug::ProfileResult &result);
    uint64_t getNewId();
    void updateAndClear();
    
    [[nodiscard]] bool isFrozen() const;
    void setFreeze(bool isFrozen);
    void setUpdateRate(float updateRate);
    [[nodiscard]] const std::vector<debug::ProfileNode> &getTree() const;

    void beginSnapshot(const std::string &filePath);
    void endSnapshot();

protected:
    void writeProfile(const debug::ProfileResult &result);
    void createTree();

    std::vector<debug::ProfileResult> mResults { };
    std::vector<debug::ProfileResult> mSnapshotResults { };
    std::vector<debug::ProfileNode> mTree { };
    std::ofstream mOutputSteam;
    float mUpdateRate { 0.1f };
    float mTimer { 0.f };
    bool mIsFrozen { false };
    uint64_t mProfileCount { 0 };
    std::string mSnapshotFilePath;
    bool mIsRecordingSnapshot { false };
    
    uint64_t mId { 0 };
};

