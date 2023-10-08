/**
 * @file Lighting.h
 * @author Ryan Purse
 * @date 08/10/2023
 */


#pragma once

#include "Pch.h"
#include "TextureArrayObject.h"

namespace graphics
{
    struct DirectionalLight
    {
        glm::vec3                               direction       { glm::normalize(glm::vec3(1.f, 1.f, 1.f)) };
        glm::vec3                               colourIntensity { 10'000.f };
        
        // Shadow Settings
        
        std::shared_ptr<TextureArrayObject>     shadowMap       { nullptr };
        
        // VP matrices are updated in the render loop. They do not need to be set.
        std::vector<glm::mat4>                  vpMatrices;
        float                                   shadowZMultiplier { 5.f };
        std::vector<float>                      shadowCascadeMultipliers { 0.04f, 0.16f, 0.36f, 0.64f };
        uint32_t                                shadowCascadeZones { 5 };
        glm::vec2                               shadowBias { 0.001f, 0.f };
        std::vector<float>                      cascadeDepths;
    };
    
    struct PointLight
    {
        glm::vec3   position        { 0.f };
        glm::vec3   colourIntensity { 12'000.f };
        float       radius          { 50.f };
    };
}
