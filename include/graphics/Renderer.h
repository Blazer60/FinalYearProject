/**
 * @file Renderer.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include "CameraMatrices.h"
#include <functional>

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
    
    bool init();
    
    void submit(uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader, DrawMode renderMode, const DrawCallback &onDraw);
    
    void submit(const CameraMatrices &cameraMatrices);
    
    void render();
    
    /**
     * @returns false if debug message was failed to be setup. This is most likely due to the openGl version being
     * less than 4.3.
     * @see https://docs.gl/gl4/glDebugMessageCallback
     */
    bool debugMessageCallback(GLDEBUGPROC callback);
    
    [[nodiscard]] std::string getVersion();
}
