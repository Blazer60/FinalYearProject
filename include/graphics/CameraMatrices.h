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
class CameraMatrices
{
public:
    CameraMatrices(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix)
        : projectionMatrix(projectionMatrix), viewMatrix(viewMatrix) {}
        
    glm::mat4 projectionMatrix { glm::mat4(1.f) };
    glm::mat4 viewMatrix { glm::mat4(1.f) };
    
    void setVpMatrix(const glm::mat4 &matrix) { vpMatrix = matrix; }
    [[nodiscard]] glm::mat4 getVpMatrix() const { return vpMatrix; }
    
protected:
    glm::mat4 vpMatrix { glm::mat4(1.f) };
};
