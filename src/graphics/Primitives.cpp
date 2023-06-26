/**
 * @file Primitives.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Primitives.h"

std::unique_ptr<SubMesh> primitives::fullscreenTriangle()
{
    std::vector<BasicVertex> triangleData {
        BasicVertex(glm::vec3(-1.f, -1.f, 0.f), glm::vec2(0.f, 0.f)),
        BasicVertex(glm::vec3( 3.f, -1.f, 0.f), glm::vec2(2.f, 0.f)),
        BasicVertex(glm::vec3(-1.f,  3.f, 0.f), glm::vec2(0.f, 2.f)),
    };
    
    std::vector<uint32_t> indices { 0, 1, 2 };
    
    return std::make_unique<SubMesh>(triangleData, indices);
}
