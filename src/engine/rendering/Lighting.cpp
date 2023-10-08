/**
 * @file Lighting.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "Lighting.h"
#include "gtc/type_ptr.hpp"
#include "GraphicsState.h"
#include "gtx/euler_angles.hpp"
#include "RendererImGui.h"
#include "Ui.h"
#include "Actor.h"
#include "GraphicsLighting.h"

DirectionalLight::DirectionalLight(
    const glm::vec3 &direction, const glm::vec3 &colour, const glm::ivec2 &shadowMapSize, uint32_t cascadeZoneCount)
    : direction(direction),
      colour(colour),
      Light()
{
    mDirectionalLight.shadowMap = std::make_shared<TextureArrayObject>(shadowMapSize, cascadeZoneCount,GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder);
    mDirectionalLight.shadowMap->setBorderColour(glm::vec4(1.f));
    mDirectionalLight.vpMatrices.reserve(cascadeZoneCount);
    calculateDirection();
}

void DirectionalLight::updateLayerCount(uint32_t cascadeCount)
{
    if (mDirectionalLight.shadowMap->getLayerCount() != cascadeCount)
    {
        const glm::ivec2 size = mDirectionalLight.shadowMap->getSize();
        mDirectionalLight.shadowMap = std::make_shared<TextureArrayObject>(size, cascadeCount, GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder);
        vpMatrices.reserve(cascadeCount);
    }
}

void DirectionalLight::onDrawUi()
{
    ImGui::PushID("DirectionalLightSettings");
    if (ImGui::TreeNodeEx("Directional Light Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Destroy Component"))
            mActor->removeComponent(this);
        
        ImGui::ColorEdit3("Colour", glm::value_ptr(colour), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
        ImGui::DragFloat("Intensity (Lux)", &intensity, 10.f);
        bool changed = false;
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.f * 0.65f);
        changed |= ImGui::DragFloat("Yaw", &yaw, 0.1f); ImGui::SameLine();
        changed |= ImGui::DragFloat("Pitch", &pitch, 0.1f);
        
        if (changed)
            calculateDirection();
        
        ImGui::PopItemWidth();
        
        if (ImGui::TreeNode("Shadow Settings"))
        {
            if (ImGui::TreeNode("Cascade Depths"))
            {
                int intCascadeZones = static_cast<int>(mDirectionalLight.shadowCascadeZones);
                if (ImGui::SliderInt("Cascade Zones", &intCascadeZones, 1, 16))
                {
                    mDirectionalLight.shadowCascadeMultipliers.resize(intCascadeZones - 1, 1.f);
                    mDirectionalLight.shadowCascadeZones = intCascadeZones;
                }
                for (int i = 0; i < mDirectionalLight.shadowCascadeMultipliers.size(); ++i)
                {
                    const std::string name = "Cascade Depth " + std::to_string(i);
                    ImGui::SliderFloat(name.c_str(), &mDirectionalLight.shadowCascadeMultipliers[i], 0.f, 1.f);
                }

                ImGui::TreePop();
            }
            
            ImGui::DragFloat("Z Multiplier", &mDirectionalLight.shadowZMultiplier, 0.1f);
            ImGui::DragFloat2("Bias", glm::value_ptr(mDirectionalLight.shadowBias), 0.001f);
            
            ImGui::Checkbox("Debug Shadows Maps", &debugShadowMaps);
            if (debugShadowMaps)
                graphics::displayShadowMaps(mDirectionalLight.shadowMap.get());
            
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void DirectionalLight::calculateDirection()
{
    const float yawRadians = glm::radians(yaw);
    const float pitchRadians = glm::radians(pitch);
    
    direction = glm::yawPitchRoll(yawRadians, pitchRadians, 0.f) * glm::vec4(0.f, 0.f, -1.f, 0.f);
}

void DirectionalLight::onPreRender()
{
    mDirectionalLight.direction = direction;
    mDirectionalLight.colourIntensity = colour * intensity;
    updateLayerCount(mDirectionalLight.shadowCascadeZones);
    
    graphics::renderer->submit(mDirectionalLight);
}

void PointLight::onPreRender()
{
    mPointLight.colourIntensity = mIntensity * mColour;
    mPointLight.position = mActor->getWorldPosition();
    mPointLight.radius = mRadius;
    
    graphics::renderer->submit(mPointLight);
}

void PointLight::onDrawUi()
{
    ImGui::PushID("PointLightSettings");
    if (ImGui::TreeNodeEx("Point Light Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Destroy Component"))
            mActor->removeComponent(this);
        
        ImGui::ColorEdit3("Colour", glm::value_ptr(mColour), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
        ImGui::DragFloat("Intensity (Lum)", &mIntensity, 10.f);
        ImGui::DragFloat("Radius", &mRadius);
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}
