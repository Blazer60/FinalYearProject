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

DirectionalLight::DirectionalLight(
    const glm::vec3 &direction, const glm::vec3 &intensity, const glm::ivec2 &shadowMapSize, uint32_t cascadeZoneCount)
    : direction(direction),
      intensity(intensity),
      shadowMap(std::make_shared<TextureArrayObject>(shadowMapSize, cascadeZoneCount, GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder)),
      Light()
{
    shadowMap->setBorderColour(glm::vec4(1.f));
    vpMatrices.reserve(cascadeZoneCount);
    calculateDirection();
}

void DirectionalLight::updateLayerCount(uint32_t cascadeCount)
{
    if (shadowMap->getLayerCount() != cascadeCount)
    {
        const glm::ivec2 size = shadowMap->getSize();
        shadowMap = std::make_unique<TextureArrayObject>(size, cascadeCount, GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder);
        vpMatrices.reserve(cascadeCount);
    }
}

void DirectionalLight::onDrawUi()
{
    ImGui::PushID("DirectionalLightSettings");
    if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Intensity", glm::value_ptr(intensity), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel);
        bool changed = false;
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.f * 0.65f);
        changed |= ImGui::DragFloat("Yaw", &yaw, 0.1f); ImGui::SameLine();
        changed |= ImGui::DragFloat("Pitch", &pitch, 0.1f);
        ImGui::PopItemWidth();
        ImGui::Checkbox("Debug Shadows Maps", &debugShadowMaps);
        if (debugShadowMaps)
            graphics::displayShadowMaps(shadowMap.get());
        
        if (changed)
            calculateDirection();
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
    updateLayerCount(graphics::renderer->shadowCascadeZones);
    // todo: change directional light to submit a light weight version independent of this class.
    graphics::renderer->submit(*this);
}