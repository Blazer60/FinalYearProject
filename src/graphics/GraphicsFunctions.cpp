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

    enum class GpuError : uint16_t
    {
        NoError = 0x00,
        InvalidEnum = 0x0500,
        InvalidValue = 0x0501,
        InvalidOperation = 0x0502,
        StackOverflow = 0x503,
        StackUnderflow = 0x504,
        OutOfMemory = 0x505,
        InvalidFramebufferOperation = 0x0506,
        ContexLost = 0x507,
        TableTooLarge = 0x8031
    };

    const char* to_string(GpuError code)
    {
        switch (code)
        {
            case GpuError::NoError: return "NoError";
            case GpuError::InvalidEnum: return "InvalidEnum";
            case GpuError::InvalidValue: return "InvalidValue";
            case GpuError::InvalidOperation: return "InvalidOperation";
            case GpuError::StackOverflow: return "StackOverflow";
            case GpuError::StackUnderflow: return "StackUnderflow";
            case GpuError::OutOfMemory: return "OutOfMemory";
            case GpuError::InvalidFramebufferOperation: return "InvalidFramebufferOperation";
            case GpuError::ContexLost: return "ContexLost";
            case GpuError::TableTooLarge: return "TableTooLarge";
            default: return "unknown";
        }
    }

    void validateGpuState()
    {
        GLenum error;
        while((error = glGetError()) != GL_NO_ERROR) // To poll out of the queue.
            CRASH("The GPU is in an invalid state! Reason %", to_string(static_cast<GpuError>(error)));
    }
}
