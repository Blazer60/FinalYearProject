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

    enum class textureFormat : uint8_t
    {
        Rgba16f, Rg16f, R16f
    };

    enum class pixelFormat : uint8_t
    {
        Red, Rg, Rgb, Rgba, Depth, Stencil
    };

    enum class gbuffer : uint8_t
    {
        Normal, Roughness, Diffuse, Specular, Emissive, ByteCount
    };
    
    GLint toGLint(filter f);
    GLint toMagGLint(filter f);
    GLint toGLint(wrap w);
    GLenum toGLenum(textureFormat f);
    GLenum toGLenum(pixelFormat p);
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