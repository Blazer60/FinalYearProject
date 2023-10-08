/**
 * @file HdrTexture.cpp
 * @author Ryan Purse
 * @date 11/07/2023
 */


#include <filesystem>
#include "HdrTexture.h"
#include "stb_image.h"

HdrTexture::HdrTexture(std::string_view path)
{
    stbi_set_flip_vertically_on_load(true);
    if (path.empty())
        return;
    
    std::filesystem::path systemPath(path);
    if (!std::filesystem::exists(systemPath))
    {
        WARN("File % does not exist.\nAborting texture generation", systemPath);
        return;
    }
    
    glCreateTextures(GL_TEXTURE_2D, 1, &mId);
    
    int width;
    int height;
    int colourChannels;
    float *data = stbi_loadf(systemPath.string().c_str(), &width, &height, &colourChannels, 0);
    
    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    const unsigned int levels = 1;
    const int lod = 0;
    const int xOffSet = 0;
    const int yOffSet = 0;
    
    glTextureStorage2D(mId, levels, GL_RGB16F, width, height);
    
    glTextureSubImage2D(mId, lod, xOffSet, yOffSet, width, height, GL_RGB, GL_FLOAT, data);
    
    stbi_image_free(data);
}

HdrTexture::~HdrTexture()
{
    if (mId != 0)
        glDeleteTextures(1, &mId);
}

void HdrTexture::setDebugName(std::string_view name)
{
    glObjectLabel(GL_TEXTURE, mId, -1, name.data());
}
