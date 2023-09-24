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

struct Light
{

};

/** A light source that is considered to be infinitely far away, such as the sun. */
struct DirectionalLight
    : public Light, public engine::Component
{
    DirectionalLight(
        const glm::vec3 &direction, const glm::vec3 &colour, const glm::ivec2 &shadowMapSize,
        uint32_t cascadeZoneCount);
    
    void updateLayerCount(uint32_t cascadeCount);
    
    glm::vec3 direction { glm::normalize(glm::vec3(1.f, 1.f, 1.f)) };
    
    glm::vec3 colour { 1.f };
    
    /**
     * @brief The colour of the light.
     */
    float intensity { 10000.f };
    
    std::shared_ptr<TextureArrayObject> shadowMap { nullptr };
    
    std::vector<glm::mat4> vpMatrices;
    
protected:
    void onDrawUi() override;
    void onPreRender() override;
    
protected:
    float yaw   { 45.f };
    float pitch { 45.f };
    bool  debugShadowMaps { false };
    
    void calculateDirection();
};

struct PointLight
    : public Light, public engine::Component
{
public:
    PointLight() = default;
    
protected:
    void onPreRender() override;
    void onDrawUi() override;

protected:
    float mRadius       { 10.f };
    float mIntensity    { 12'000.f };
    glm::vec3 mColour   { 1.f };
};
