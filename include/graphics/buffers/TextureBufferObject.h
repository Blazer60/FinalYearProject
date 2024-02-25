/**
 * @file TextureBufferObject.h
 * @author Ryan Purse
 * @date 14/03/2022
 */


#pragma once

#include "Pch.h"
#include <functional>
#include "GraphicsDefinitions.h"

/**
 * Contains a texture that can be read and written to by openGl.
 * Most commonly used by attaching itself to framebuffer objects.
 * @author Ryan Purse
 * @date 14/03/2022
 */
class TextureBufferObject
{
public:
    explicit TextureBufferObject(const glm::ivec2 &size);
    explicit TextureBufferObject(TextureBufferObject &tex) = delete;
    
    /**
     * @see <a href="https://www.khronos.org/opengl/wiki/Image_Format">Image Formatting</a>
     * @param format - The OpenGL format with the syntax GL_(components)(size)[type].
     */
    TextureBufferObject(
        const glm::ivec2 &size, GLenum format, GLint minFilter, GLint magFilter, uint32_t mipmapLevels = 1);
    
    TextureBufferObject(
        const glm::ivec2 &size, GLenum format, graphics::filter filterMode, graphics::wrap wrapMode,
        uint32_t mipmapLevel = 1);
    
    virtual ~TextureBufferObject();
    
    [[nodiscard]] unsigned int getId() const;
    [[nodiscard]] const glm::ivec2 &getSize() const;
    void setBorderColour(const glm::vec4 &colour) const;
    [[nodiscard]] GLenum getFormat() const;
    void generateMipMaps() const;
    [[nodiscard]] uint32_t getMipLevels() const;
    void clear(const glm::vec4 clearColour=glm::vec4(0.f, 0.f, 0.f, 1.f)) const;
    void setDebugName(const std::string &debugName) const;
protected:
    void init(GLint minFilter, GLint magFilter, GLint wrapS, GLint wrapT);
    void deInit();
    
    unsigned int    mId           { 0 };
    GLenum          mFormat         { GL_RGB16 };
    glm::ivec2      mSize           { 1024 };
    uint32_t        mMipMapLevels   { 1 };
};
