/**
 * @file Renderer.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Renderer.h"
#include "FramebufferObject.h"
#include "WindowHelpers.h"

#include <utility>

namespace renderer
{
    std::vector<RenderQueueObject> renderQueue;
    std::vector<CameraSettings> cameraQueue;
    GLenum drawModeToGLenum[] { GL_TRIANGLES, GL_LINES };
    std::unique_ptr<FramebufferObject> geometryFramebuffer;
    std::unique_ptr<TextureBufferObject> outputTextureBuffer;
    std::unique_ptr<RenderBufferObject> geometryRenderbuffer;
    
    bool init()
    {
        if (glewInit() != GLEW_OK)
            return false;
        
        // Blending texture data / enabling lerping.
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        
        geometryFramebuffer  = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);
        outputTextureBuffer  = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16, GL_NEAREST, GL_NEAREST, 1, "Output");
        geometryRenderbuffer = std::make_unique<RenderBufferObject>(window::bufferSize());
        
        geometryFramebuffer->attach(outputTextureBuffer.get(), 0);
        geometryFramebuffer->attach(geometryRenderbuffer.get());
        
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
    
    void drawMesh(
        uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader, DrawMode renderMode, const glm::mat4 &matrix,
        const DrawCallback &onDraw)
    {
        GLenum mode = drawModeToGLenum[(int)renderMode];
        renderQueue.emplace_back(RenderQueueObject { vao, indicesCount, std::move(shader), mode, matrix, onDraw });
    }
    
    
    void drawMesh(const SubMesh &subMesh, Material &material, const glm::mat4 &matrix)
    {
        drawMesh(
            subMesh.vao(), subMesh.indicesCount(), material.shader(), material.drawMode(), matrix,
            [&]() { material.onDraw(); }
        );
    }
    
    void drawMesh(const SharedMesh &mesh, const SharedMaterials &materials, const glm::mat4 &matrix)
    {
        if (materials.size() == 1)
        {
            Material& material = *materials[0];
            
            for (const auto &subMesh : mesh)
                drawMesh(*subMesh, material, matrix);
        }
        else
        {
            for (int i = 0; i < mesh.size(); ++i)
            {
                SubMesh& subMesh = *mesh[i];
                Material& material = *materials[i];
                drawMesh(subMesh, material, matrix);
            }
        }
    }
    
    void submit(const CameraSettings &cameraMatrices)
    {
        cameraQueue.emplace_back(cameraMatrices);
    }
    
    void render()
    {
        for (CameraSettings &camera : cameraQueue)
        {
            geometryFramebuffer->bind();
            geometryFramebuffer->clear(camera.clearColour);
            
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
        
        uint64_t renderQueueCount = renderQueue.size();
        renderQueue.clear();
        renderQueue.reserve(renderQueueCount);
        
        uint64_t cameraCount = cameraQueue.size();
        cameraQueue.clear();
        cameraQueue.reserve(cameraCount);
    }
    
    const TextureBufferObject &getOutputBuffer()
    {
        return *outputTextureBuffer;
    }
}

