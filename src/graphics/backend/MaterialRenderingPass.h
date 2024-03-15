/**
 * @file MaterialRendering.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "CameraSettings.h"
#include "Context.h"
#include "FileLoader.h"
#include "GraphicsDefinitions.h"
#include "MaterialData.h"
#include "Pch.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 09/03/2024
     */
    class MaterialRenderingPass
    {
    public:
        void execute(const glm::ivec2 &size, Context &context, const std::vector<RenderQueueObject> &renderQueue);
        void execute(const glm::ivec2 &size, Context &context, const std::vector<GeometryObject> &geometryQueue, const std::vector<MaterialData> &materials);
    protected:
        FramebufferObject mFramebuffer = FramebufferObject(GL_ONE, GL_ZERO, GL_LESS);

        Shader mMaterialShader {
            {
                file::shaderPath() / "geometry/standard/Standard.vert",
                file::shaderPath() / "geometry/standard/Material.frag"
            },
            {
                { "FRAGMENT_OUTPUT", 1 }
            }
        };

        ShaderStorageBufferObject mMaterialShaderStorage = ShaderStorageBufferObject("Material Shader Storage");
        ShaderStorageBufferObject mTextureDataShaderStorage = ShaderStorageBufferObject("Texture Data Shader Storage");
        ShaderStorageBufferObject mMaskShaderStorage = ShaderStorageBufferObject("Mask Shader Storage");
    };
} // graphics
