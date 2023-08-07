/**
 * @file RendererImGui.cpp
 * @author Ryan Purse
 * @date 03/07/2023
 */


#include "RendererImGui.h"
#include "imgui.h"
#include "Ui.h"
#include "gtc/type_ptr.hpp"
#include "PostProcessLayer.h"
#include "GraphicsState.h"
#include "GraphicsFunctions.h"
#include "Buffers.h"

namespace graphics
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
                int intCascadeZones = static_cast<int>(graphics::renderer->shadowCascadeZones);
                if (ImGui::SliderInt("Cascade Zones", &intCascadeZones, 1, 16))
                {
                    graphics::renderer->shadowCascadeMultipliers.resize(intCascadeZones - 1, 1.f);
                    graphics::renderer->shadowCascadeZones = intCascadeZones;
                }
                for (int i = 0; i < graphics::renderer->shadowCascadeMultipliers.size(); ++i)
                {
                    const std::string name = "Cascade Depth " + std::to_string(i);
                    ImGui::SliderFloat(name.c_str(), &graphics::renderer->shadowCascadeMultipliers[i], 0.f, 1.f);
                }
                
                ImGui::TreePop();
            }
            
            ImGui::DragFloat("Z Multiplier", &graphics::renderer->shadowZMultiplier, 0.1f);
            ImGui::DragFloat2("Bias", glm::value_ptr(graphics::renderer->shadowBias), 0.001f);
        }
    }
    
    void displayShadowMaps(TextureArrayObject *textureArrayObject)
    {
        ImGui::SliderInt("Shadow map layer", &layer, 0, textureArrayObject->getLayerCount() - 1);
        shadowMapViewer = graphics::cloneTextureLayer(*textureArrayObject, layer);
        bool yes = true;
        ui::showTextureBuffer("Shadow Map", *shadowMapViewer, &yes, shadowMapViewer->getSize());
    }
}
