/**
 * @file RendererHelpers.h
 * @author Ryan Purse
 * @date 20/06/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include <functional>

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
    
    GLint toGLint(filter f);
    GLint toGLint(wrap w);
    
    struct RenderQueueObject
    {
        uint32_t vao;
        int32_t indicesCount;
        std::weak_ptr<Shader> shader;
        GLenum drawMode;
        glm::mat4 matrix;
        DrawCallback onDraw;
    };
    
    /**
     * @brief Graphics Point Light. Named like this to not interfere with the engine point light.
     */
    struct AnalyticalPointLight
    {
        glm::vec3 position { 0.f };
        float radius       { 10.f };
        float intensity    { 12'000.f };
        glm::vec3 colour   { 1.f };
    };
}