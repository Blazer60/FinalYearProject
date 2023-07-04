/**
 * @file RendererImGui.cpp
 * @author Ryan Purse
 * @date 03/07/2023
 */


#include "RendererImGui.h"
#include "ShadowMapping.h"
#include "RendererData.h"
#include "imgui.h"
#include "Ui.h"
#include "gtc/type_ptr.hpp"

namespace renderer
{
    bool showShadowMaps { false };
    std::unique_ptr<TextureBufferObject> shadowMapViewer;
    int layer { 0 };
    
    void displayShadowSettings()
    {
        if (ImGui::CollapsingHeader("Shadow Settings"))
        {
            if (ImGui::TreeNode("Cascade Depths"))
            {
                int intCascadeZones = static_cast<int>(shadow::cascadeZones);
                if (ImGui::SliderInt("Cascade Zones", &intCascadeZones, 1, 16))
                {
                    shadow::cascadeMultipliers.resize(intCascadeZones - 1, 1.f);
                    shadow::cascadeZones = intCascadeZones;
                }
                for (int i = 0; i < shadow::cascadeMultipliers.size(); ++i)
                {
                    const std::string name = "Cascade Depth " + std::to_string(i);
                    ImGui::SliderFloat(name.c_str(), &shadow::cascadeMultipliers[i], 0.f, 1.f);
                }
                
                ImGui::TreePop();
            }
            
            ImGui::DragFloat("Z Multiplier", &shadow::zMultiplier, 0.1f);
            ImGui::DragFloat2("Bias", glm::value_ptr(shadow::bias), 0.001f);
            ImGui::Checkbox("Debug shadow maps", &showShadowMaps);
            
            if (showShadowMaps)
            {
                ImGui::SliderInt("Shadow map layer", &layer, 0, directionalLightQueue[0].shadowMap->getLayerCount() - 1);
                shadowMapViewer = renderer::cloneTextureLayer(*directionalLightQueue[0].shadowMap, layer);
                ui::showTextureBuffer("Shadow Map", *shadowMapViewer, &showShadowMaps, shadowMapViewer->getSize());
            }
        }
    }
    
}
