/**
 * @file RendererImpl.h
 * @author Ryan Purse
 * @date 03/07/2023
 */


#pragma once

#include "Pch.h"
#include "RendererHelpers.h"
#include "Cubemap.h"
#include "FramebufferObject.h"
#include "Mesh.h"
#include "Lighting.h"

namespace renderer
{
    extern glm::ivec2 currentRenderBufferSize;
    
    extern std::unique_ptr<Cubemap> skybox;
    
    extern std::unique_ptr<FramebufferObject> deferredLightFramebuffer;
    extern std::unique_ptr<FramebufferObject> geometryFramebuffer;
    extern std::unique_ptr<FramebufferObject> lightFramebuffer;
    extern std::unique_ptr<FramebufferObject> shadowFramebuffer;
    
    extern std::unique_ptr<Shader> deferredLightShader;
    extern std::unique_ptr<Shader> directionalLightShader;
    extern std::unique_ptr<Shader> shadowShader;
    
    extern std::unique_ptr<SubMesh> fullscreenTriangle;
    
    extern std::unique_ptr<TextureBufferObject> albedoTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> depthTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> diffuseTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> emissiveTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> normalTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> outputTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> positionTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> shadowTextureBuffer;
    extern std::unique_ptr<TextureBufferObject> specularTextureBuffer;
    
    extern std::vector<CameraSettings> cameraQueue;
    extern std::vector<DirectionalLight> directionalLightQueue;
    extern std::vector<RenderQueueObject> renderQueue;
}
