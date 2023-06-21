/**
 * @file RenderBufferObject.h
 * @author Ryan Purse
 * @date 14/03/2022
 */


#pragma once

#include "Pch.h"

/**
 * Holds information about the format and how it should be attached to a framebuffer.
 * @author Ryan Purse
 * @date 14/03/2022
 */
class RenderBufferObject
{
public:
    explicit RenderBufferObject(const glm::ivec2 &size);
    ~RenderBufferObject();
    
    void resize(const glm::ivec2 &size) const;
    
    [[nodiscard]] unsigned int getName() const;
    [[nodiscard]] GLenum getAttachment() const;
protected:
    
    void init();
    void deInit();
    unsigned int    mId       { 0 };
    
    glm::ivec2      mSize       { 1024 };
    GLenum          mFormat     { GL_DEPTH24_STENCIL8 };
    GLenum          mAttachment { GL_DEPTH_STENCIL_ATTACHMENT };
};


