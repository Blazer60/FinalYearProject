/**
 * @file RendererFunctions.cpp
 * @author Ryan Purse
 * @date 15/07/2023
 */


#include "GraphicsFunctions.h"

namespace graphics
{
    std::unique_ptr<TextureBufferObject> cloneTextureLayer(const TextureArrayObject &from, int layer)
    {
        auto result = std::make_unique<TextureBufferObject>(from.getSize(), from.getFormat(), filter::Linear, wrap::Repeat);
        const int x = 0;
        const int y = 0;
        const int z = 0;
        const int mipLevel = 0;
        glCopyImageSubData(from.getId(), GL_TEXTURE_2D_ARRAY, mipLevel, x, y, layer, result->getId(), GL_TEXTURE_2D, mipLevel, x, y, z, result->getSize().x, result->getSize().y, 1);
        return result;
    }
    
    void copyTexture2D(const TextureBufferObject &source, const TextureBufferObject &destination)
    {
        constexpr int x = 0;
        constexpr int y = 0;
        constexpr int z = 0;
        constexpr int mipLevel = 0;
        constexpr int sourceDepth = 1;
        glCopyImageSubData(
            source.getId(), GL_TEXTURE_2D, mipLevel, x, y, z,
            destination.getId(), GL_TEXTURE_2D, mipLevel, x, y, z,
            destination.getSize().x, destination.getSize().y, sourceDepth);
    }

    void copyTexture2D(const Texture& source, const TextureArrayObject& destination, const int destinationIndex)
    {
        constexpr int x = 0;
        constexpr int y = 0;
        constexpr int z = 0;
        constexpr int mipLevel = 0;
        constexpr int sourceDepth = 1;

        glCopyImageSubData(
            source.id(), GL_TEXTURE_2D, mipLevel, x, y, z,
            destination.getId(), GL_TEXTURE_2D_ARRAY, mipLevel, x, y, destinationIndex,
            source.size().x, source.size().y, sourceDepth
            );
    }

    void pushDebugGroup(std::string_view message)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, message.data());
    }
    
    void popDebugGroup()
    {
        glPopDebugGroup();
    }

    void setViewport(const glm::ivec2 size)
    {
        glViewport(0, 0, size.x, size.y);
    }

    void dispatchCompute(const glm::uvec3 size)
    {
        glDispatchCompute(size.x, size.y, size.z);
    }

    void dispatchCompute(const glm::uvec2 size)
    {
        glDispatchCompute(size.x, size.y, 1);
    }

    void dispatchCompute(const uint32_t size)
    {
        glDispatchCompute(size, 1, 1);
    }

    void dispatchComputeIndirect(const uint32_t buffer, const int offset)
    {
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, buffer);
        glDispatchComputeIndirect(static_cast<GLintptr>(offset));
    }
}
