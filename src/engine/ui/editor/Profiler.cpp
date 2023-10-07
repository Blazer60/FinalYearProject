/**
 * @file ProfileResults.cpp
 * @author Ryan Purse
 * @date 07/10/2023
 */


#include "Profiler.h"
#include "Timers.h"

namespace engine
{
    void Profiler::addResult(const engine::ProfileResult &result)
    {
        mResults.push_back(result);
    }
    
    void Profiler::onDrawUi()
    {
        if (ImGui::Begin("Profiler"))
        {
#ifndef ENABLE_PROFILING
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Profiling has been disabled in the build settings.");
#else
            ImGui::Checkbox("Freeze", &mIsFrozen);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Update Rate").x);
            ImGui::SliderFloat("Update Rate", &mUpdateRate, 0.f, 1.f);
            for (const auto &node : mTree)
                drawNode(node);
#endif
            ImGui::End();
        }
    }
    
    void Profiler::createTree()
    {
        std::sort(mResults.begin(), mResults.end(), [](const ProfileResult &lhs, const ProfileResult &rhs) {
            return lhs.startMicroSeconds < rhs.startMicroSeconds;
        });
        
        for (const auto &item : mResults)
        {
            bool isInserted = false;
            for (auto &node : mTree)
                isInserted |= node.tryInsert(item);
            
            if (!isInserted)
                mTree.push_back({ item.name, item.startMicroSeconds, item.stopMicroSeconds, {} });
        }
    }
    
    void Profiler::drawNode(const ProfileNode &node)
    {
        const float time = static_cast<float>((node.stopNanoSeconds - node.startNanoSeconds)) * 0.001f * 0.001f;
        std::string label = node.name.data();
        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen;
        if (node.children.empty())
            treeFlags |= ImGuiTreeNodeFlags_Leaf;
        
        if (ImGui::TreeNodeEx(label.c_str(), treeFlags, "%.3fms | %s", time, node.name.data()))
        {
            for (const auto &child : node.children)
                drawNode(child);
            ImGui::TreePop();
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
    }
    
    bool ProfileNode::tryInsert(ProfileResult result)
    {
        if (result.startMicroSeconds >= startNanoSeconds && result.stopMicroSeconds <= stopNanoSeconds)
        {
            bool isInserted = false;
            for (ProfileNode &child : children)
                isInserted |= child.tryInsert(result);
            
            if (!isInserted)
                children.push_back({ result.name, result.startMicroSeconds, result.stopMicroSeconds, { } });
            
            return true;
        }
        return false;
    }
}