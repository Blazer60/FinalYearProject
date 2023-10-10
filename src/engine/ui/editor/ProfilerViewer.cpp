/**
 * @file ProfilerViewer.cpp
 * @author Ryan Purse
 * @date 08/10/2023
 */


#include "ProfilerViewer.h"
#include "Profiler.h"

namespace engine
{
    void ProfilerViewer::onDrawUi()
    {
        if (ImGui::Begin("Profiler"))
        {
#ifndef ENABLE_PROFILING
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Profiling has been disabled in the build settings.");
#else
            bool isFrozen = profiler->isFrozen();
            ImGui::Checkbox("Freeze", &isFrozen);
            profiler->setFreeze(isFrozen);
            
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Update Rate").x);
            ImGui::SliderFloat("Update Rate", &mUpdateRate, 0.f, 1.f);
            profiler->setUpdateRate(mUpdateRate);
            for (const auto &node : profiler->getTree())
                drawNode(node);
#endif
            ImGui::End();
        }
    }
    
    void ProfilerViewer::drawNode(const debug::ProfileNode &node)
    {
        const float time = static_cast<float>((node.stopNanoSeconds - node.startNanoSeconds)) * 0.001f * 0.001f;
        std::string label = node.name.data() + std::to_string(node.id);
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
}

