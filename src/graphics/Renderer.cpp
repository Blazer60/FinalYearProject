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
    std::vector<CameraSettings> cameraQueue;
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
        uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader, DrawMode renderMode, const glm::mat4 &matrix,
        const DrawCallback &onDraw)
    {
        GLenum mode = drawModeToGLenum[(int)renderMode];
        renderQueue.emplace_back(RenderQueueObject { vao, indicesCount, std::move(shader), mode, matrix, onDraw });
    }
    
    void submit(const CameraSettings &cameraMatrices)
    {
        cameraQueue.emplace_back(cameraMatrices);
    }
    
    void render()
    {
        for (CameraSettings &camera : cameraQueue)
        {
            glClearColor(camera.clearColour.r, camera.clearColour.g, camera.clearColour.b, camera.clearColour.a);
            glClear(camera.clearMask);
            
            glm::mat4 vpMatrix = camera.projectionMatrix * camera.viewMatrix;
            
            for (const auto &rqo : renderQueue)
            {
                if (rqo.shader.expired())
                    continue;
                
                const auto shader = rqo.shader.lock();
                shader->bind();
                shader->set("u_mvp_matrix", vpMatrix * rqo.matrix);
                rqo.onDraw();
                glBindVertexArray(rqo.vao);
                glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
            }
        }
        
        uint64_t count = renderQueue.size();
        renderQueue.clear();
        renderQueue.reserve(count);
    }
    
    void renderer::submit(const SubMesh &subMesh, Material &material, const glm::mat4 &matrix)
    {
        submit(subMesh.vao(), subMesh.indicesCount(), material.shader(), material.drawMode(), matrix, [&]() { material.onDraw(); });
    }
    
    void renderer::submit(const SharedMesh &mesh, const SharedMaterials &materials, const glm::mat4 &matrix)
    {
        if (materials.size() == 1)
        {
            Material& material = *materials[0];
            
            for (const auto &subMesh : mesh)
                submit(*subMesh, material, matrix);
        }
        else
        {
            for (int i = 0; i < mesh.size(); ++i)
            {
                SubMesh& subMesh = *mesh[i];
                Material& material = *materials[i];
                submit(subMesh, material, matrix);
            }
        }
    }
    
}

