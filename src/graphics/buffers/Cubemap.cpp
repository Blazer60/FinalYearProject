/**
 * @file Cubemap.cpp
 * @author Ryan Purse
 * @date 26/06/2023
 */


#include <filesystem>
#include "Cubemap.h"
#include "stb_image.h"
#include <Statistics.h>

Cubemap::Cubemap(const std::vector<std::string> &paths)
{
    stbi_set_flip_vertically_on_load(0);
    if (paths.size() != 6)
    {
        WARN("You're trying to create a cubemap with an incorrect amount of faces. No cubemap will be generated.");
        return;
    }
    
    for (const std::string_view path : paths)
    {
        if (const std::filesystem::path systemPath(path); !exists(systemPath))
        {
            WARN("File % does not exist.\nNo cubemap will be generated.", systemPath);
            return;
        }
    }
    
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);
    
    int initialWidth  = 0;
    int initialHeight = 0;
    
    for (int i = 0; i < paths.size(); ++i)
    {
        const std::string_view path = paths[i];
        
        const std::filesystem::path systemPath(path);
        
        int width = 0;
        int height = 0;
        int colourChannels = 0;
        // Forcing 4 components so that is always aligns with opengl's internal format.
        unsigned char *bytes = stbi_load(systemPath.string().c_str(), &width, &height, &colourChannels, 4);


        constexpr int lod = 0;
        constexpr int xOffSet = 0;
        constexpr int yOffSet = 0;
        const int zOffSet = i;
        constexpr int depth = 1;
        
        if (i == 0)
        {
            constexpr unsigned int levels = 1;
            initialWidth = width;
            initialHeight = height;
            glTextureStorage2D(mId, levels, GL_RGBA8, width, height);
        }
        else if (width != initialWidth || initialHeight != height)
        {
            WARN("File % does not match the size of the first texture. Cubemaps textures must be all the same size.",
                 systemPath);
            
            stbi_image_free(bytes);
            glDeleteTextures(1, &mId);
            mId = 0;
            return;
        }
        
        if (colourChannels > 4)
        {
            WARN("File % does not contain the correct amount of channels. Cannot generate textures", systemPath);
            
            stbi_image_free(bytes);
            glDeleteTextures(1, &mId);
            mId = 0;
            return;
        }
        
        glTextureSubImage3D(mId, lod, xOffSet, yOffSet, zOffSet, width, height, depth, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
        
        stbi_image_free(bytes);
    }

    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    mSize = { initialWidth, initialHeight };
    mFormat = GL_RGBA8;
}

void Cubemap::init()
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);

    glTextureStorage2D(mId, mMipLevels, mFormat, mSize.x, mSize.y);

    if (mMipLevels > 1)
        glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    else
        glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (mMipLevels > 1)
        glGenerateTextureMipmap(mId);
}

Cubemap::Cubemap(const glm::ivec2 &size, const GLenum format, const int mipLevels)
    : mSize(size), mFormat(format), mMipLevels(mipLevels)
{
    init();
}

Cubemap::Cubemap(const graphics::textureFormat format)
    : mFormat(toGLenum(format))
{

}

Cubemap::Cubemap(const graphics::textureFormat format, const int mipLevels)
    : mFormat(toGLenum(format)), mMipLevels(mipLevels)
{
}

void Cubemap::deInit()
{
    if (mId != 0)
        glDeleteTextures(1, &mId);
    mId = 0;
}

Cubemap::~Cubemap()
{
    deInit();
}

void Cubemap::resize(const glm::ivec2 newSize)
{
    if (mSize == newSize)
        return;
    deInit();
    mSize = newSize;
    init();
}

void Cubemap::setDebugName(const std::string_view name) const
{
    glObjectLabel(GL_TEXTURE, mId, -1, name.data());
}

