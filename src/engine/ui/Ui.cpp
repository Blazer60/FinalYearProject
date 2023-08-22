/**
 * @file UiHelpers.cpp
 * @author Ryan Purse
 * @date 27/06/2023
 */


#include "Ui.h"
#include "imgui.h"
#include "WindowHelpers.h"

namespace ui
{
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, bool isMainBuffer)
    {
        if (show && !*show)
            return;
        
        ImGui::PushID(name.c_str());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        if (ImGui::Begin(name.c_str(), show))
        {
            ImVec2 regionSize = ImGui::GetContentRegionAvail();
            
            if (isMainBuffer)
                window::setBufferSize(glm::ivec2(regionSize.x, regionSize.y));
            
            ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, const glm::ivec2 &size, bool fitToRegion)
    {
        if (show && !*show)
            return;
        
        ImGui::PushID(name.c_str());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        if (ImGui::Begin(name.c_str(), show))
        {
            ImVec2 regionSize = ImGui::GetContentRegionAvail();
            
            if (fitToRegion)
            {
                if (size.x == size.y)
                {
                    if (regionSize.x > regionSize.y)
                    {
                        if (static_cast<float>(size.y) >= regionSize.y)
                        {
                            const float ratio = regionSize.y / static_cast<float>(size.y);
                            regionSize.x = ratio * static_cast<float>(size.y);
                        }
                    }
                    else
                    {
                        if (static_cast<float>(size.x) >= regionSize.x)
                        {
                            const float ratio = regionSize.x / static_cast<float>(size.x);
                            regionSize.y = ratio * static_cast<float>(size.y);
                        }
                    }
                }
                if (size.x > size.y)
                {
                    if (static_cast<float>(size.x) >= regionSize.x)
                    {
                        const float ratio = regionSize.x / static_cast<float>(size.x);
                        regionSize.y = ratio * static_cast<float>(size.y);
                    }
                }
                else
                {
                    if (static_cast<float>(size.y) >= regionSize.y)
                    {
                        const float ratio = regionSize.y / static_cast<float>(size.y);
                        regionSize.x = ratio * static_cast<float>(size.y);
                    }
                }
            }
            
            ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    
    void drawToolTip(std::string_view message, float tooltipWidth)
    {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * tooltipWidth);
            ImGui::Text("%s", message.data());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}
