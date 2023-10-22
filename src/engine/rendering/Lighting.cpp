/**
 * @file Lighting.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "Lighting.h"

#include <utility>
#include "gtc/type_ptr.hpp"
#include "GraphicsState.h"
#include "gtx/euler_angles.hpp"
#include "RendererImGui.h"
#include "Ui.h"
#include "Actor.h"
#include "GraphicsLighting.h"
#include "EngineState.h"
#include "Editor.h"
#include "FileExplorer.h"

namespace engine
{
    DirectionalLight::DirectionalLight(
        float yaw, float pitch, const glm::vec3 &colour, float intensity, const std::vector<float> &depths,
        float zMultiplier, const glm::vec2 &bias)
        :
        colour(colour), yaw(yaw), pitch(pitch), intensity(intensity), Light()
    {
        const glm::ivec2 shadowMapSize = glm::ivec2(2048);
        mDirectionalLight.shadowMap = std::make_shared<TextureArrayObject>(shadowMapSize, static_cast<int32_t >(depths.size()), GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder);
        mDirectionalLight.shadowMap->setBorderColour(glm::vec4(1.f));
        mDirectionalLight.vpMatrices.reserve(depths.size());
        mDirectionalLight.shadowBias = bias;
        mDirectionalLight.shadowCascadeMultipliers = depths;
        mDirectionalLight.shadowZMultiplier = zMultiplier;
        calculateDirection();
    }
    
    DirectionalLight::DirectionalLight(
        const glm::vec3 &direction, const glm::vec3 &colour, const glm::ivec2 &shadowMapSize, uint32_t cascadeZoneCount)
        : direction(direction),
          colour(colour),
          Light()
    {
        mDirectionalLight.shadowMap = std::make_shared<TextureArrayObject>(shadowMapSize, cascadeZoneCount, GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder);
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
        
        if (mPointLight.position != mActor->getWorldPosition())
            computeVpMatrices();
        
        mPointLight.position = mActor->getWorldPosition();
        mPointLight.radius = mRadius;
        mPointLight.bias = mBias;
        mPointLight.softnessRadius = mSoftnessRadius;
        
        if (mPointLight.shadowMap->getSize().x != mResolution)
            mPointLight.shadowMap = std::make_shared<Cubemap>(glm::ivec2(mResolution), GL_DEPTH_COMPONENT32);
        
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
            if (ImGui::DragFloat("Radius", &mRadius))
                computeVpMatrices();
            
            ImGui::DragFloat2("Bias", glm::value_ptr(mBias), 0.001f);
            ImGui::DragFloat("Softness", &mSoftnessRadius, 0.001f);
            ImGui::DragInt("Resolution", &mResolution, 0.f, 32, 8192, "%d", ImGuiSliderFlags_Logarithmic);
            
            ImGui::TreePop();
        }
        
        ImGui::PopID();
    }

    PointLight::PointLight()
    {
        mPointLight.shadowMap = std::make_shared<Cubemap>(glm::ivec2(1024), GL_DEPTH_COMPONENT32);
    }
    
    PointLight::PointLight(
        const glm::vec3 &colour, float intensity, float radius,
        const glm::vec2 &bias, float softness, int resolution)
        :
        mRadius(radius), mIntensity(intensity), mColour(colour), mBias(bias), mSoftnessRadius(softness),
        mResolution(resolution)
    {
        mPointLight.shadowMap = std::make_shared<Cubemap>(glm::ivec2(resolution), GL_DEPTH_COMPONENT32);
    }

    void PointLight::computeVpMatrices()
    {
        const glm::mat4 viewRotations[] = {
            glm::lookAt(glm::vec3(0.f), glm::vec3( 1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)),
            glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)),
            glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f, -1.f,  0.f), glm::vec3(0.f,  0.f, -1.f)),
            glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  1.f,  0.f), glm::vec3(0.f,  0.f,  1.f)),
            glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  0.f,  1.f), glm::vec3(0.f, -1.f,  0.f)),
            glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  0.f, -1.f), glm::vec3(0.f, -1.f,  0.f)),
        };
        
        const glm::mat4 lightModelMatrix = getWorldTransform();
        const glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.f), 1.f, 0.01f, mRadius);
        
        for (int i = 0; i < 6; ++i)
        {
            const glm::mat4 viewMatrix = glm::inverse(lightModelMatrix * viewRotations[i]);
            const glm::mat4 vpMatrix = projectionMatrix * viewMatrix;
            mPointLight.vpMatrices[i] = vpMatrix;
        }
    }
    
    DistantLightProbe::DistantLightProbe(const std::filesystem::path &path, const glm::ivec2 &size)
        : mPath(path), mSize(size), mThumbnailTexture(std::make_unique<Texture>(path))
    {
    
    }
    
    DistantLightProbe::DistantLightProbe(const glm::ivec2 &size)
        : mSize(size), mThumbnailTexture(std::make_unique<Texture>(""))
    {
    
    }
    
    void DistantLightProbe::onPreRender()
    {
        graphics::renderer->setIblMultiplier(mRadianceMultiplier);
        if (!mIsUpdated)
        {
            if (!mPath.empty())
                graphics::renderer->generateSkybox(mPath.string(), mSize);
            mIsUpdated = true;
        }
    }
    
    void DistantLightProbe::onDrawUi()
    {
        ImGui::PushID("DistantLightProbeSettings");
        if (ImGui::TreeNodeEx("Distant Light Probe", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);
            
            ImGui::DragFloat("Radiance Multiplier", &mRadianceMultiplier);
            ImGui::SliderInt2("Size", glm::value_ptr(mSize), 32, 4096, "%d");
            if (ImGui::Button("Update"))
                mIsUpdated = false;
            
            if (ui::imageButton("HDR Texture", mThumbnailTexture->id(), ui::fitToRegion(mThumbnailTexture->size(), glm::ivec2(512))))
            {
                engine::editor->addUpdateAction([this]() {
                    const std::string result = openFileExplorer();
                    if (result.empty())
                        return;
                    
                    mPath = result;
                    mThumbnailTexture = std::make_unique<Texture>(result);
                    mIsUpdated = false;
                });
            }
            
            ImGui::TreePop();
        }
        
        ImGui::PopID();
    }
}
