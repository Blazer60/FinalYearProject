/**
 * @file FrameBufferObject.h
 * @author Ryan Purse
 * @date 14/03/2022
 */


#pragma once

#include "Pch.h"
#include "TextureBufferObject.h"
#include "RenderBufferObject.h"

#include <glm.hpp>

/**
 * A set of details on where to render to for OpenGL.
 * @author Ryan Purse
 * @date 14/03/2022
 */
class FramebufferObject
{
public:
    explicit FramebufferObject();
    
    /**
     * @brief Creates a framebuffer with a depth texture already attached to it.
     * Blending equation: result = fragColour * sourceFunction + bufferColour * destinationColour.
     * @param sourceBlendFunction - The amount of frag colour that will go through.
     * @param destinationBlendFunction - The amount of buffer colour that will go through.
     * @param depthFunction - Flag on how to pass the depth test (GL_ALWAYS to disable depth testing).
     */
    FramebufferObject(GLenum sourceBlendFunction, GLenum destinationBlendFunction, GLenum depthFunction);
    
    ~FramebufferObject();
    
    void attach(const TextureBufferObject *textureBufferObject, int bindPoint, int mipLevel = 0);
    void attach(const RenderBufferObject *renderBufferObject) const;
    
    void detach(int bindPoint);
    void detachRenderBuffer() const;
    void clear(const glm::vec4 &clearColour);
    void bind() const;
    
    [[nodiscard]] unsigned int getFboName() const;
protected:
    void validate() const;
    
    std::vector<GLenum> mAttachments;
    
    unsigned int    mFboId        { 0 };
    const float     mDepth        { 1.f };
    
    GLenum mSourceBlend         { GL_ONE };
    GLenum mDestinationBlend    { GL_ZERO };
    GLenum mDepthFunction       { GL_LESS };
};


