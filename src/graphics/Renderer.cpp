/**
 * @file Renderer.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Renderer.h"

#include <utility>

namespace renderer
{
    std::vector<RenderQueueObject> renderQueue;
    std::vector<CameraMatrices> cameraQueue;
    GLenum drawModeToGLenum[] { GL_TRIANGLES, GL_LINES };
    
    bool init()
    {
        if (glewInit() != GLEW_OK)
            return false;
        
        // Blending texture data / enabling lerping.
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        
        return true;
    }
    
    bool debugMessageCallback(GLDEBUGPROC callback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(callback, nullptr);
        
        int flags { 0 };  // Check to see if OpenGL debug context was set up correctly.
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        
        return (flags & GL_CONTEXT_FLAG_DEBUG_BIT);
    }
    
    std::string getVersion()
    {
        return (reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    }
    
    void submit(
        uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader, renderer::DrawMode renderMode,
        const renderer::DrawCallback &onDraw)
    {
        GLenum mode = drawModeToGLenum[(int)renderMode];
        renderQueue.emplace_back(RenderQueueObject { vao, indicesCount, std::move(shader), mode, onDraw });
    }
    
    void submit(const CameraMatrices &cameraMatrices)
    {
        cameraQueue.emplace_back(cameraMatrices);
    }
    
    void render()
    {
        for (CameraMatrices &camera : cameraQueue)
        {
            glClearColor(1.f, 1.f, 1.f, 1.f);  // todo: should these be part of the camera settings?
            glClear(GL_COLOR_BUFFER_BIT);
            
            camera.setVpMatrix(camera.projectionMatrix * camera.viewMatrix);
            
            for (const auto &rqo : renderQueue)
            {
                if (rqo.shader.expired())
                    continue;
                
                const auto shader = rqo.shader.lock();
                shader->bind();
                rqo.onDraw(*shader, camera);
                glBindVertexArray(rqo.vao);
                glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
            }
        }
        
        uint64_t count = renderQueue.size();
        renderQueue.clear();
        renderQueue.reserve(count);
    }
    
}

