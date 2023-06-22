/**
 * @file Texture.cpp
 * @author Ryan Purse
 * @date 22/06/2023
 */


#include <filesystem>
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(std::string_view path)
{
    stbi_set_flip_vertically_on_load(true);
    if (path.empty())
        return;
    
    std::filesystem::path systemPath(path);
    if (!std::filesystem::exists(systemPath))
    {
        debug::log("File " + systemPath.string() + " does not exist.\nAborting texture generation",
                   debug::severity::Warning);
        return;
    }
    
    glCreateTextures(GL_TEXTURE_2D, 1, &mId);
    
    int width;
    int height;
    int colourChannels;
    // Forcing 4 components so that is always aligns with opengl's internal format.
    unsigned char *bytes = stbi_load(systemPath.string().c_str(), &width, &height, &colourChannels, 4);
    
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    const unsigned int levels = 1;
    const int lod = 0;
    const int xOffSet = 0;
    const int yOffSet = 0;
    
    glTextureStorage2D(mId, levels, GL_RGBA8, width, height);
    
    if (colourChannels > 4)
    {
        debug::log("File " + systemPath.string()
                   + " does not contain the correct amount of channels. Cannot generate textures",
                   debug::severity::Warning);
        stbi_image_free(bytes);
        glDeleteTextures(1, &mId);
        mId = 0;
        return;
    }
    
    glTextureSubImage2D(mId, lod, xOffSet, yOffSet, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    glGenerateTextureMipmap(mId);
    
    stbi_image_free(bytes);
}

Texture::~Texture()
{
    if (mId != 0)
        glDeleteTextures(1, &mId);
}
