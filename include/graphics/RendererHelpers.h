/**
 * @file RendererHelpers.h
 * @author Ryan Purse
 * @date 20/06/2023
 */


#pragma once

#include "Pch.h"
#include <functional>
#include "Shader.h"
#include "CameraMatrices.h"

namespace renderer
{
    using DrawCallback = std::function<void(Shader&, const CameraMatrices&)>;
    
    enum DrawMode
    {
        Triangles, Lines
    };
    
    struct RenderQueueObject
    {
        uint32_t vao;
        int32_t indicesCount;
        std::weak_ptr<Shader> shader;
        GLenum drawMode;
        DrawCallback onDraw;
    };
}