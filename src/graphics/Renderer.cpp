/**
 * @file Renderer.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Renderer.h"
#include "FramebufferObject.h"
#include "WindowHelpers.h"
#include "Primitives.h"
#include "Cubemap.h"
#include "RendererData.h"
#include "ShadowMapping.h"

#include <utility>

namespace renderer
{
    bool init()
    {
        if (glewInit() != GLEW_OK)
            return false;
        
        // Blending texture data / enabling lerping.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        
        directionalLightShader = std::make_unique<Shader>("../resources/shaders/FullscreenTriangle.vert", "../resources/shaders/lighting/DirectionalLight.frag");
        deferredLightShader = std::make_unique<Shader>("../resources/shaders/FullscreenTriangle.vert", "../resources/shaders/lighting/CombineOutput.frag");
        shadowShader = std::make_unique<Shader>("../resources/shaders/shadow/Shadow.vert", "../resources/shaders/shadow/Shadow.frag");
        fullscreenTriangle = primitives::fullscreenTriangle();
        skybox = std::make_unique<Cubemap>(std::vector<std::string> {
            "../resources/textures/skybox/right.jpg",
            "../resources/textures/skybox/left.jpg",
            "../resources/textures/skybox/top.jpg",
            "../resources/textures/skybox/bottom.jpg",
            "../resources/textures/skybox/front.jpg",
            "../resources/textures/skybox/back.jpg",
        });
        
        initFrameBuffers();
        initTextureRenderBuffers();
        
        currentRenderBufferSize = window::bufferSize();
        glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
        
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
        static GLenum drawModeToGLenum[] { GL_TRIANGLES, GL_LINES };
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
    
    void submit(const DirectionalLight &directionalLight)
    {
        directionalLightQueue.emplace_back(directionalLight);
    }
    
    void render()
    {
        if (currentRenderBufferSize != window::bufferSize())
        {
            detachTextureRenderBuffersFromFrameBuffers();
            initTextureRenderBuffers();
            glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
            currentRenderBufferSize = window::bufferSize();
        }
        
        for (CameraSettings &camera : cameraQueue)
        {
            geometryFramebuffer->bind();
            geometryFramebuffer->clear(camera.clearColour);
            
            const glm::mat4 cameraProjectionMatrix = glm::perspective(camera.fovY, window::aspectRatio(), camera.nearClipDistance, camera.farClipDistance);
            const glm::mat4 vpMatrix = cameraProjectionMatrix * camera.viewMatrix;
            
            for (const auto &rqo : renderQueue)
            {
                if (rqo.shader.expired())
                    continue;
                
                const auto shader = rqo.shader.lock();
                shader->bind();
                shader->set("u_mvp_matrix", vpMatrix * rqo.matrix);
                shader->set("u_model_matrix", rqo.matrix);
                rqo.onDraw();
                glBindVertexArray(rqo.vao);
                glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
            }
            
            // Shadow mapping
            
            const std::vector<float> cascadeDepths { camera.farClipDistance * shadow::cascadeMultipliers[0], camera.farClipDistance * shadow::cascadeMultipliers[1] };
            
            shadowMapping(camera, cascadeDepths);
            
            lightFramebuffer->bind();
            lightFramebuffer->clear(glm::vec4(glm::vec3(0.f), 1.f));
            
            directionalLightShader->bind();
            
            directionalLightShader->set("u_albedo_texture", albedoTextureBuffer->getId(), 0);
            directionalLightShader->set("u_position_texture", positionTextureBuffer->getId(), 1);
            directionalLightShader->set("u_normal_texture", normalTextureBuffer->getId(), 2);
            
            const glm::vec3 cameraPosition = glm::inverse(camera.viewMatrix) * glm::vec4(glm::vec3(0.f), 1.f);
            directionalLightShader->set("u_camera_position_ws", cameraPosition);
            directionalLightShader->set("u_view_matrix", camera.viewMatrix);
            
            directionalLightShader->set("u_cascade_distances", &(cascadeDepths[0]), static_cast<int>(cascadeDepths.size()));
            directionalLightShader->set("u_cascade_count", static_cast<int>(cascadeDepths.size()));
            
            glBindVertexArray(fullscreenTriangle->vao());
            
            for (const DirectionalLight &directionalLight : directionalLightQueue)
            {
                directionalLightShader->set("u_light_direction", directionalLight.direction);
                directionalLightShader->set("u_light_intensity", directionalLight.intensity);
                directionalLightShader->set("u_light_vp_matrix", &(directionalLight.vpMatrices[0]), static_cast<int>(directionalLight.vpMatrices.size()));
                directionalLightShader->set("u_shadow_map_texture", directionalLight.shadowMap->getId(), 3);
                
                glDrawElements(GL_TRIANGLES, fullscreenTriangle->indicesCount(), GL_UNSIGNED_INT, nullptr);
            }
            
            // Deferred Lighting step.
            
            deferredLightFramebuffer->bind();
            deferredLightFramebuffer->clear(glm::vec4(glm::vec3(0.f), 0.f));
            
            deferredLightShader->bind();
            
            const glm::mat4 v = glm::mat4(glm::mat3(camera.viewMatrix));
            const glm::mat4 vp = cameraProjectionMatrix * v;
            const glm::mat4 ivp = glm::inverse(vp);
            
            deferredLightShader->set("u_diffuse_texture", diffuseTextureBuffer->getId(), 0);
            deferredLightShader->set("u_specular_texture", specularTextureBuffer->getId(), 1);
            deferredLightShader->set("u_albedo_texture", albedoTextureBuffer->getId(), 2);
            deferredLightShader->set("u_emissive_texture", emissiveTextureBuffer->getId(), 3);
            deferredLightShader->set("u_depth_texture", depthTextureBuffer->getId(), 4);
            deferredLightShader->set("u_skybox_texture", skybox->getId(), 5);
            deferredLightShader->set("u_shadow_texture", shadowTextureBuffer->getId(), 6);
            deferredLightShader->set("u_inverse_vp_matrix", ivp);
            
            glBindVertexArray(fullscreenTriangle->vao());  // I know it's still bound but just it's just to avoid future errors.
            glDrawElements(GL_TRIANGLES, fullscreenTriangle->indicesCount(), GL_UNSIGNED_INT, nullptr);
        }
        
        
    }

    
    const TextureBufferObject &getOutputBuffer()
    {
        return *outputTextureBuffer;
    }
    
    void initFrameBuffers()
    {
        // One, Zero (override any geometry that is further away from the camera).
        geometryFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);
        
        // One, One (additive blending for each light that we pass through)
        lightFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ONE, GL_ALWAYS);
        
        // We only ever write to this framebuffer once, so it shouldn't matter.
        deferredLightFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ONE, GL_ALWAYS);
        
        shadowFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);
    }
    
    void initTextureRenderBuffers()
    {
        positionTextureBuffer   = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST, 1, "Position Buffer");
        albedoTextureBuffer     = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST, 1, "Albedo Buffer");
        normalTextureBuffer     = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16_SNORM,           GL_NEAREST, GL_NEAREST, 1, "Normal Buffer");
        emissiveTextureBuffer   = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST, 1, "Emissive Buffer");
        diffuseTextureBuffer    = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST, 1, "Diffuse Buffer");
        specularTextureBuffer   = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST, 1, "Specular Buffer");
        outputTextureBuffer     = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST, 1, "Output Buffer");
        shadowTextureBuffer     = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_R16F,                  GL_NEAREST, GL_NEAREST, 1, "Shadow Buffer");
        depthTextureBuffer      = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_DEPTH_COMPONENT32F,    GL_NEAREST, GL_NEAREST, 1, "Depth Buffer");
        
        // Make sure that the framebuffers have been set up before calling this function.
        geometryFramebuffer->attach(positionTextureBuffer.get(),    0);
        geometryFramebuffer->attach(normalTextureBuffer.get(),      1);
        geometryFramebuffer->attach(albedoTextureBuffer.get(),      2);
        geometryFramebuffer->attach(emissiveTextureBuffer.get(),    3);
        
        geometryFramebuffer->attachDepthBuffer(depthTextureBuffer.get());
        
        // Lighting.
        lightFramebuffer->attach(diffuseTextureBuffer.get(), 0);
        lightFramebuffer->attach(specularTextureBuffer.get(), 1);
        lightFramebuffer->attach(shadowTextureBuffer.get(), 2);
        
        // Deferred Lighting.
        deferredLightFramebuffer->attach(outputTextureBuffer.get(), 0);
    }
    
    void detachTextureRenderBuffersFromFrameBuffers()
    {
        geometryFramebuffer->detach(0);
        geometryFramebuffer->detach(1);
        geometryFramebuffer->detach(2);
        geometryFramebuffer->detach(3);
        geometryFramebuffer->detachDepthBuffer();
        
        lightFramebuffer->detach(0);
        lightFramebuffer->detach(1);
        lightFramebuffer->detach(2);
        
        deferredLightFramebuffer->detach(0);
    }
    
    const TextureBufferObject &getAlbedoBuffer()
    {
        return *albedoTextureBuffer;
    }
    
    const TextureBufferObject &getNormalBuffer()
    {
        return *normalTextureBuffer;
    }
    
    const TextureBufferObject &getPositionBuffer()
    {
        return *positionTextureBuffer;
    }
    
    const TextureBufferObject &getEmissiveBuffer()
    {
        return *emissiveTextureBuffer;
    }
    
    const TextureBufferObject &getDiffuseBuffer()
    {
        return *diffuseTextureBuffer;
    }
    
    const TextureBufferObject &getSpecularBuffer()
    {
        return *specularTextureBuffer;
    }
    
    const TextureBufferObject &getDepthBuffer()
    {
        return *depthTextureBuffer;
    }
    
    const TextureBufferObject &getShadowBuffer()
    {
        return *shadowTextureBuffer;
    }
    
    void clear()
    {
        uint64_t renderQueueCount = renderQueue.size();
        renderQueue.clear();
        renderQueue.reserve(renderQueueCount);
        
        uint64_t cameraCount = cameraQueue.size();
        cameraQueue.clear();
        cameraQueue.reserve(cameraCount);
        
        uint64_t directionalLightCount = directionalLightQueue.size();
        directionalLightQueue.clear();
        directionalLightQueue.reserve(directionalLightCount);
    }
}

