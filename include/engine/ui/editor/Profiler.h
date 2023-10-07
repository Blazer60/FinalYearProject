/**
 * @file ProfileResults.h
 * @author Ryan Purse
 * @date 07/10/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"


namespace engine
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
    
    /**
     * @author Ryan Purse
     * @date 07/10/2023
     */
    class Profiler
        : public ui::Drawable
    {
    public:
        void addResult(const ProfileResult &result);
        void updateAndClear();
    
    protected:
        void onDrawUi() override;
        void createTree();
        
        void drawNode(const ProfileNode &node);
    
    protected:
        std::vector<ProfileResult> mResults;
        std::vector<ProfileNode> mTree;
        float mUpdateRate { 0.1f };
        float mTimer { 0.f };
        bool mIsFrozen { false };
    };
}

