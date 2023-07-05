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

struct Light
{

};

/** A light source that is considered to be infinitely far away, such as the sun. */
struct DirectionalLight
    : public Light
{
    DirectionalLight(
        const glm::vec3 &direction, const glm::vec3 &intensity, const glm::ivec2 &shadowMapSize,
        uint32_t cascadeZoneCount)
        : direction(direction),
        intensity(intensity),
        shadowMap(std::make_shared<TextureArrayObject>(shadowMapSize, cascadeZoneCount, GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder)),
        Light()
    {
        shadowMap->setBorderColour(glm::vec4(1.f));
        vpMatrices.reserve(cascadeZoneCount);
    }
    
    void updateLayerCount(uint32_t cascadeCount)
    {
        if (shadowMap->getLayerCount() != cascadeCount)
        {
            const glm::ivec2 size = shadowMap->getSize();
            shadowMap = std::make_unique<TextureArrayObject>(size, cascadeCount, GL_DEPTH_COMPONENT32, graphics::filter::Linear, graphics::wrap::ClampToBorder);
            vpMatrices.reserve(cascadeCount);
        }
    }
    
    glm::vec3 direction { glm::normalize(glm::vec3(1.f, 1.f, 1.f)) };
    
    /**
     * @brief The colour of the light.
     */
    glm::vec3 intensity { 1.f };
    
    std::shared_ptr<TextureArrayObject> shadowMap { nullptr };
    
    std::vector<glm::mat4> vpMatrices;
};
