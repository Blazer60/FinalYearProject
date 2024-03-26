/**
 * @file Texture.cpp
 * @author Ryan Purse
 * @date 22/06/2023
 */


#include "Texture.h"

#include <Statistics.h>
#include <filesystem>

#include "FileLoader.h"

Texture::Texture(const std::filesystem::path &path)
    : mPath(path)
{

}

Texture::~Texture()
{
    if (mId != 0)
        glDeleteTextures(1, &mId);
}

void Texture::setData(const glm::ivec2 &size, const unsigned char* bytes)
{
    if (mId != 0)
        glDeleteTextures(1, &mId);
    mId = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &mId);

    glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    mSize = size;

    const unsigned int levels = 1;
    const int lod = 0;
    const int xOffSet = 0;
    const int yOffSet = 0;

    glTextureStorage2D(mId, levels, GL_RGBA8, size.x, size.y);
    glTextureSubImage2D(mId, lod, xOffSet, yOffSet, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    glGenerateTextureMipmap(mId);
}
