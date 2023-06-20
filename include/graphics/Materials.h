/**
 * @file Materials.h
 * @author Ryan Purse
 * @date 15/06/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include "RendererHelpers.h"

class Material
{
public:
    explicit Material(renderer::DrawMode drawMode)
        : mDrawMode(drawMode) {};
    
    [[nodiscard]] renderer::DrawMode drawMode() const { return mDrawMode; }
    
    virtual void OnDraw(Shader &shader) {};
protected:
    renderer::DrawMode mDrawMode;
};

class SimpleMaterial
    : public Material
{
public:
    SimpleMaterial() : Material(renderer::Triangles) { };
    
    void OnDraw(Shader &shader) override
    {
        shader.set("u_colour", glm::vec3(0.f, 0.f, 1.f));
    }
};
