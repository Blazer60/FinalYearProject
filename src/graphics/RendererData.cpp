/**
 * @file RendererImpl.cpp
 * @author Ryan Purse
 * @date 03/07/2023
 */


#include "RendererData.h"

namespace renderer
{
    glm::ivec2 currentRenderBufferSize;
    
    std::unique_ptr<Cubemap> skybox;
    
    std::unique_ptr<FramebufferObject> deferredLightFramebuffer;
    std::unique_ptr<FramebufferObject> geometryFramebuffer;
    std::unique_ptr<FramebufferObject> lightFramebuffer;
    std::unique_ptr<FramebufferObject> shadowFramebuffer;
    
    std::unique_ptr<Shader> deferredLightShader;
    std::unique_ptr<Shader> directionalLightShader;
    std::unique_ptr<Shader> shadowShader;
    
    std::unique_ptr<SubMesh> fullscreenTriangle;
    
    std::unique_ptr<TextureBufferObject> albedoTextureBuffer;
    std::unique_ptr<TextureBufferObject> depthTextureBuffer;
    std::unique_ptr<TextureBufferObject> diffuseTextureBuffer;
    std::unique_ptr<TextureBufferObject> emissiveTextureBuffer;
    std::unique_ptr<TextureBufferObject> normalTextureBuffer;
    std::unique_ptr<TextureBufferObject> outputTextureBuffer;
    std::unique_ptr<TextureBufferObject> positionTextureBuffer;
    std::unique_ptr<TextureBufferObject> shadowTextureBuffer;
    std::unique_ptr<TextureBufferObject> specularTextureBuffer;
    
    std::vector<CameraSettings> cameraQueue;
    std::vector<DirectionalLight> directionalLightQueue;
    std::vector<RenderQueueObject> renderQueue;
}
