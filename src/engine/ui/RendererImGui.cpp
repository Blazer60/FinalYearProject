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

namespace renderer
{
    bool showShadowMaps { false };
    std::unique_ptr<TextureBufferObject> shadowMapViewer;
    int layer { 0 };
    
    void displayShadowSettings()
    {
        if (ImGui::CollapsingHeader("Shadow Settings"))
        {
            ImGui::SliderFloat("Cascade Depth 1", &shadow::cascadeMultipliers[0], 0.f, 1.f);
            ImGui::SliderFloat("Cascade Depth 2", &shadow::cascadeMultipliers[1], 0.f, 1.f);
            ImGui::DragFloat("Z Multiplier", &shadow::zMultiplier, 0.1f);
            if (ImGui::Button("Toggle Shadow maps"))
                showShadowMaps = !showShadowMaps;
            
            if (showShadowMaps)
            {
                ImGui::SliderInt("Shadow map layer", &layer, 0, directionalLightQueue[0].shadowMap->getLayerCount() - 1);
                shadowMapViewer = renderer::cloneTextureLayer(*directionalLightQueue[0].shadowMap, layer);
                ui::showTextureBuffer("Shadow Map", *shadowMapViewer, &showShadowMaps, shadowMapViewer->getSize());
            }
        }
    }
    
}
