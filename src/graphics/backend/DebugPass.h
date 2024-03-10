/**
 * @file DebugPass.h
 * @author Ryan Purse
 * @date 10/03/2024
 */


#pragma once

#include "Context.h"
#include "DebugGBufferBlock.h"
#include "FileLoader.h"
#include "Mesh.h"
#include "Pch.h"
#include "TextureBufferObject.h"
#include "../Primitives.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 10/03/2024
     */
    class DebugPass 
    {
    public:
        DebugPass();
        void execute(const glm::ivec2 &size, const Context &context, const std::vector<DebugQueueObject> &debugQueue, const std::vector<LineQueueObject> &lineQueue);
        TextureBufferObject &tileOverlay(const glm::ivec2 &size, const Context &context);
        TextureBufferObject &whiteFurnaceTest(const glm::ivec2 &size, const Context &context);
        TextureBufferObject &queryGBuffer(
            const glm::ivec2 &size, const Context& context,
            gbuffer type, bool gammaCorrect, const glm::vec4& defaultValue);
        const TextureBufferObject& getDebugOverlay();

    protected:
        Shader mDebugGBufferShader {
            { file::shaderPath() / "geometry/DebugGBuffer.comp" }
        };

        Shader mWhiteFurnaceTestShader {
            { file::shaderPath() / "lighting/IBL.comp" },
            { { "WHITE_FURNACE_TEST" } }
        };

        Shader mDebugTileOverlayShader {
            { file::shaderPath() / "classification/DebugOverlay.comp" }
        };

        Shader mDebugShader {
            { file::shaderPath() / "geometry/debug/Debug.vert", file::shaderPath() / "geometry/debug/Debug.frag" }
        };

        Shader mLineShader {
            { file::shaderPath() / "geometry/debug/Line.vert", file::shaderPath() / "geometry/debug/Line.frag" }
        };

        FramebufferObject mDebugFramebuffer = FramebufferObject(GL_ONE, GL_ZERO, GL_LESS);

        UniformBufferObject<DebugGBufferBlock> mDebugGBufferBlock;

        TextureBufferObject mDebugTexture = TextureBufferObject(textureFormat::Rgba16f);
        TextureBufferObject mDebugOverlay = TextureBufferObject(textureFormat::Rgba16f);

        SubMesh mLine = primitives::line();
    };
} // graphics
