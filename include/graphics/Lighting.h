/**
 * @file Lighting.h
 * @author Ryan Purse
 * @date 22/06/2023
 */


#pragma once

#include "Pch.h"
#include "vec3.hpp"
#include "geometric.hpp"
#include "TextureArrayObject.h"
#include "ShadowMapping.h"

struct Light
{

};

/** A light source that is considered to be infinitely far away, such as the sun. */
struct DirectionalLight
    : public Light
{
    DirectionalLight(const glm::vec3 &direction, const glm::vec3 &intensity, const glm::ivec2 &shadowMapSize)
        : direction(direction),
        intensity(intensity),
        shadowMap(std::make_shared<TextureArrayObject>(shadowMapSize, renderer::shadow::cascadeZones, GL_DEPTH_COMPONENT32, renderer::Linear, renderer::ClampToBorder)),
        Light()
    {
        shadowMap->setBorderColour(glm::vec4(1.f));
        vpMatrices.reserve(renderer::shadow::cascadeZones);
    }
    
    void updateLayerCount()
    {
        if (shadowMap->getLayerCount() != renderer::shadow::cascadeZones)
        {
            const glm::ivec2 size = shadowMap->getSize();
            shadowMap = std::make_unique<TextureArrayObject>(size, renderer::shadow::cascadeZones, GL_DEPTH_COMPONENT32, renderer::Linear, renderer::ClampToBorder);
            vpMatrices.reserve(renderer::shadow::cascadeZones);
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
