/**
 * @file Lighting.h
 * @author Ryan Purse
 * @date 22/06/2023
 */


#pragma once

#include "Pch.h"
#include "vec3.hpp"
#include "geometric.hpp"

struct Light
{
    explicit Light(std::shared_ptr<TextureBufferObject> shadowMap)
        : shadowMap(std::move(shadowMap))
    {
        this->shadowMap->setBorderColour(glm::vec4(1.f));
    }
        
    // This is a shared pointer as we have to copy it to the renderer.
    std::shared_ptr<TextureBufferObject> shadowMap = nullptr;
};

/** A light source that is considered to be infinitely far away, such as the sun. */
struct DirectionalLight
    : public Light
{
    DirectionalLight(const glm::vec3 &direction, const glm::vec3 &intensity, const glm::ivec2 &shadowMapSize)
        : direction(direction),
        intensity(intensity),
        Light(std::make_shared<TextureBufferObject>(shadowMapSize, GL_DEPTH_COMPONENT32, renderer::Nearest, renderer::ClampToBorder))
    {
    
    }
    
    glm::vec3 direction { glm::normalize(glm::vec3(1.f, 1.f, 1.f)) };
    
    /**
     * @brief The colour of the light.
     */
    glm::vec3 intensity { 1.f };
};
