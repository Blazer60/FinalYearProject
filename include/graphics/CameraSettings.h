/**
 * @file CameraMatrices.h
 * @author Ryan Purse
 * @date 14/06/2023
 */


#pragma once

#include "Pch.h"
#include "glm.hpp"
#include "PostProcessLayer.h"

/**
 * @author Ryan Purse
 * @date 14/06/2023
 */
class CameraSettings
{
public:
    CameraSettings(float fovY, float nearClipDistance, float farClipDistance, const glm::mat4 &viewMatrix, std::vector<std::unique_ptr<PostProcessLayer>> &postProcessStack)
        : fovY(fovY), nearClipDistance(nearClipDistance), farClipDistance(farClipDistance), viewMatrix(viewMatrix), postProcessStack(postProcessStack)
    {
    
    }
    
    float fovY { 60.f };
    float nearClipDistance { 0.001f };
    float farClipDistance  { 100.f };
    
    glm::mat4 viewMatrix { glm::mat4(1.f) };
    glm::vec4 clearColour { glm::vec4(0.f, 0.f, 0.f, 1.f) };
    GLbitfield clearMask { GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT };
    
    // todo: Camera exposure should be here rather than on one of the post-processing settings.
    
    std::vector<std::unique_ptr<PostProcessLayer>> &postProcessStack;
};
