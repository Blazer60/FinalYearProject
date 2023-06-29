/**
 * @file Cubemap.cpp
 * @author Ryan Purse
 * @date 26/06/2023
 */


#include <filesystem>
#include "Cubemap.h"
#include "stb_image.h"

Cubemap::Cubemap(const std::vector<std::string> &paths)
{
    stbi_set_flip_vertically_on_load(false);
    if (paths.size() != 6)
    {
        debug::log("You're trying to create a cubemap with an incorrect amount of faces. No cubemap will be generated.",
                   debug::severity::Warning);
        return;
    }
    
    for (std::string_view path : paths)
    {
        std::filesystem::path systemPath(path);
        if (!std::filesystem::exists(systemPath))
        {
            debug::log("File " + systemPath.string() + " does not exist.\nNo cubemap will be generated.",
                       debug::severity::Warning);
            return;
        }
    }
    
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);
    
    int initialWidth  = 0;
    int initialHeight = 0;
    
    for (int i = 0; i < paths.size(); ++i)
    {
        std::string_view path = paths[i];
        
        std::filesystem::path systemPath(path);
        
        int width;
        int height;
        int colourChannels;
        // Forcing 4 components so that is always aligns with opengl's internal format.
        unsigned char *bytes = stbi_load(systemPath.string().c_str(), &width, &height, &colourChannels, 4);
        
        
        const unsigned int levels = 1;
        const int lod = 0;
        const int xOffSet = 0;
        const int yOffSet = 0;
        const int zOffSet = i;
        const int depth = 1;
        
        if (i == 0)
        {
            initialWidth = width;
            initialHeight = height;
            glTextureStorage2D(mId, levels, GL_RGBA8, width, height);
        }
        else if (width != initialWidth || initialHeight != height)
        {
            debug::log("File " + systemPath.string() + " does not match the size of the first texture. Cubemaps textures "
                                                       "must be all the same size.", debug::severity::Warning);
            stbi_image_free(bytes);
            glDeleteTextures(1, &mId);
            mId = 0;
            return;
        }
        
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
        
        glTextureSubImage3D(mId, lod, xOffSet, yOffSet, zOffSet, width, height, depth, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
        // glTextureSubImage2D(mId, lod, xOffSet, yOffSet, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
        
        stbi_image_free(bytes);
    }
    
    // glGenerateTextureMipmap(mId);
    
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Cubemap::~Cubemap()
{
    if (mId != 0)
        glDeleteTextures(1, &mId);
}