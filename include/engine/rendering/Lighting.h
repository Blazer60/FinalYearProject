/**
 * @file Lighting.h
 * @author Ryan Purse
 * @date 22/06/2023
 */


#pragma once

#include "Pch.h"
#include "vec3.hpp"
#include "geometric.hpp"
#include "Buffers.h"
#include "Component.h"
#include "GraphicsLighting.h"

namespace engine
{

    struct Light
    {

    };

    /** A light source that is considered to be infinitely far away, such as the sun. */
    struct DirectionalLight
        : public Light, public engine::Component
    {
        DirectionalLight(float yaw, float pitch, const glm::vec3 &colour, float intensity, const std::vector<float> &depths, float zMultiplier, const glm::vec2 &bias);
        DirectionalLight(
            const glm::vec3 &direction, const glm::vec3 &colour, const glm::ivec2 &shadowMapSize,
            uint32_t cascadeZoneCount);
        ~DirectionalLight() override = default;
        
    protected:
        void updateShadowMap(uint32_t cascadeCount);
        void onDrawUi() override;
        void onPreRender() override;
        
    protected:
        glm::vec3 direction { glm::normalize(glm::vec3(1.f, 1.f, 1.f)) };
        
        glm::vec3 colour { 1.f };
        
    /**
    * @brief The colour of the light.
    */
        float intensity { 10000.f };
        
        std::vector<glm::mat4> vpMatrices;
        float yaw   { 45.f };
        float pitch { 45.f };
        bool  debugShadowMaps { false };
        graphics::DirectionalLight mDirectionalLight;
        int mShadowMapSize { 2048 };

        void calculateDirection();
        
        ENGINE_SERIALIZABLE_COMPONENT(DirectionalLight);
    };

    struct PointLight
        : public Light, public engine::Component
    {
    public:
        PointLight();
        PointLight(const glm::vec3 &colour, float intensity, float radius, const glm::vec2 &bias, float softness, int resolution);
        ~PointLight() override = default;
        
    protected:
        void onPreRender() override;
        void onDrawUi() override;
        void computeVpMatrices();

    protected:
        float mRadius           { 30.f };
        float mIntensity        { 12'000.f };
        glm::vec3 mColour       { 1.f };
        glm::vec2 mBias         { 0.005f, 0.15f };
        float mSoftnessRadius   { 0.02f };
        int mResolution         { 1024 };
        
        graphics::PointLight mPointLight;
        
        ENGINE_SERIALIZABLE_COMPONENT(PointLight);
    };
    
    struct SpotLight
        : Light, engine::Component
    {
        SpotLight();
    
    protected:
        void onDrawUi() override;
        void onPreRender() override;
        
        graphics::Spotlight mSpotlight;
        glm::vec3 mColour        { 1.f };
        float mIntensity         { 120'000.f };
        float mInnerAngleDegrees { 22.5f };
        float mOuterAngleDegrees { 45.f };
        float mPitch             { -90.f };
        float mYaw               { 0.f };
        bool  mUseActorRotation  { true };
        bool  mDebugShadowMap    { false };
        
        void calculateDirection();
    };

    struct DistantLightProbe
        : Light, engine::Component
    {
    public:
        explicit DistantLightProbe(const glm::ivec2 &size);
        DistantLightProbe(const std::filesystem::path &path, const glm::ivec2 &size);
        
    protected:
        void onPreRender() override;
        void onDrawUi() override;
        
    protected:
        glm::ivec2 mSize;
        float mRadianceMultiplier { 1.f };
        std::filesystem::path mPath;
        std::unique_ptr<Texture> mThumbnailTexture;
        bool mIsUpdated { false };
    };
}
