/**
 * @file RendererHelpers.h
 * @author Ryan Purse
 * @date 20/06/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include <functional>
#include <Statistics.h>

namespace graphics
{
    using DrawCallback = std::function<void()>;
    
    enum class drawMode
    {
        Triangles, Lines
    };
    
    enum class filter
    {
        Nearest, Linear, NearestMipmapNearest, LinearMipmapNearest, NearestMipmapLinear, LinearMipmapLinear
    };
    
    enum class wrap
    {
        ClampToEdge, ClampToBorder, MirrorRepeat, Repeat, MirrorClampToEdge
    };

    enum class gbuffer : uint8_t
    {
        Normal, Roughness, Diffuse, Specular, Emissive
    };
    
    GLint toGLint(filter f);
    GLint toMagGLint(filter f);
    GLint toGLint(wrap w);
    int toInt(gbuffer g);
    
    struct RenderQueueObject
    {
        uint32_t vao;
        int32_t indicesCount;
        std::weak_ptr<Shader> shader;
        GLenum drawMode;
        glm::mat4 matrix;
        DrawCallback onDraw;
    };

    struct DebugQueueObject
    {
        uint32_t    vao;
        int32_t     indiciesCount;
        glm::mat4   matrix;
        glm::vec3   colour;
    };

    struct LineQueueObject
    {
        glm::vec3 startPosition;
        glm::vec3 endPosition;
        glm::vec3 colour;
    };
}