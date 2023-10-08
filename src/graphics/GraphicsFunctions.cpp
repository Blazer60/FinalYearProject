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
        const int x = 0;
        const int y = 0;
        const int z = 0;
        const int mipLevel = 0;
        const int sourceDepth = 1;
        glCopyImageSubData(
            source.getId(), GL_TEXTURE_2D, mipLevel, x, y, z,
            destination.getId(), GL_TEXTURE_2D, mipLevel, x, y, z,
            destination.getSize().x, destination.getSize().y, sourceDepth);
    }
    
    void pushDebugGroup(std::string_view message)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, message.data());
    }
    
    void popDebugGroup()
    {
        glPopDebugGroup();
    }
}
