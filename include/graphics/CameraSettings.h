/**
 * @file CameraMatrices.h
 * @author Ryan Purse
 * @date 14/06/2023
 */


#pragma once

#include "Pch.h"
#include "glm.hpp"


/**
 * @author Ryan Purse
 * @date 14/06/2023
 */
class CameraSettings
{
public:
    CameraSettings(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix)
        : projectionMatrix(projectionMatrix), viewMatrix(viewMatrix) {}
        
    glm::mat4 projectionMatrix { glm::mat4(1.f) };
    glm::mat4 viewMatrix { glm::mat4(1.f) };
    glm::vec4 clearColour { glm::vec4(0.f, 0.f, 0.f, 1.f) };
    GLbitfield clearMask { GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT };
    
protected:
    glm::mat4 vpMatrix { glm::mat4(1.f) };
};
