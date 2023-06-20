/**
 * @file RendererHelpers.h
 * @author Ryan Purse
 * @date 20/06/2023
 */


#pragma once

#include "Pch.h"
#include <functional>
#include "Shader.h"
#include "CameraSettings.h"

namespace renderer
{
    using DrawCallback = std::function<void(Shader&)>;
    
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
        glm::mat4 matrix;
        DrawCallback onDraw;
    };
}