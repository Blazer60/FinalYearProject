/**
 * @file Context.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "CameraBlock.h"
#include "FramebufferObject.h"
#include "Pch.h"
#include "ShaderStorageBufferObject.h"
#include "TextureArrayObject.h"
#include "TextureBufferObject.h"
#include "UniformBufferObject.h"

namespace graphics
{
    constexpr int32_t gbufferLayers = 3;

    /**
     * A render context that holds most of the data needed during rendering.
     * @author Ryan Purse
     * @date 09/03/2024
     */
    struct Context
    {
        TextureArrayObject gbuffer = TextureArrayObject(textureFormat::Rgba32ui, gbufferLayers);

        TextureBufferObject depthBuffer = TextureBufferObject(textureFormat::D32f);
        TextureBufferObject lightBuffer = TextureBufferObject(textureFormat::Rgba16f);
        TextureBufferObject backBuffer = TextureBufferObject(textureFormat::Rgba16);

        ShaderStorageBufferObject tileClassificationStorage = ShaderStorageBufferObject(0, "Shader Storage Buffer Object");

        UniformBufferObject<CameraBlock> camera;

        glm::mat4 cameraViewProjectionMatrix;
    };
} // graphics
