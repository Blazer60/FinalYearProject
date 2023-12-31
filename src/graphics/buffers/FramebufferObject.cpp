/**
 * @file FrameBufferObject.cpp
 * @author Ryan Purse
 * @date 14/03/2022
 */


#include "FramebufferObject.h"
#include "gtc/type_ptr.hpp"
#include "TextureArrayObject.h"
#include "TextureBufferObject.h"
#include "Cubemap.h"
#include "RenderBufferObject.h"
#include "GraphicsFunctions.h"

FramebufferObject::FramebufferObject()
{
    glCreateFramebuffers(1, &mFboId);
}

FramebufferObject::FramebufferObject(GLenum sourceBlendFunction, GLenum destinationBlendFunction, GLenum depthFunction) :
    mSourceBlend(sourceBlendFunction), mDestinationBlend(destinationBlendFunction), mDepthFunction(depthFunction)
{
    glCreateFramebuffers(1, &mFboId);
}

FramebufferObject::~FramebufferObject()
{
    glDeleteFramebuffers(1, &mFboId);
    mFboId = 0;
}

void FramebufferObject::attach(const TextureBufferObject *textureBufferObject, int bindPoint, int mipLevel)
{
    glNamedFramebufferTexture(mFboId, GL_COLOR_ATTACHMENT0 + bindPoint, textureBufferObject->getId(), mipLevel);
    
    mAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + bindPoint);
    glNamedFramebufferDrawBuffers(mFboId, static_cast<int>(mAttachments.size()), &mAttachments[0]);
    validate();
}

void FramebufferObject::attach(const RenderBufferObject *const renderBufferObject) const
{
    glNamedFramebufferRenderbuffer(mFboId, renderBufferObject->getAttachment(), GL_RENDERBUFFER,
                                   renderBufferObject->getName());
    
    validate();
}

void FramebufferObject::attach(const Cubemap *cubemap, int bindPoint, int layer, int mipLevel)
{
    glNamedFramebufferTextureLayer(mFboId, GL_COLOR_ATTACHMENT0 + bindPoint, cubemap->getId(), mipLevel, layer);
    
    mAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + bindPoint);
    glNamedFramebufferDrawBuffers(mFboId, static_cast<int>(mAttachments.size()), &mAttachments[0]);
    validate();
}

void FramebufferObject::attachDepthBuffer(const TextureBufferObject *textureBufferObject, int mipLevel)
{
    if (!(textureBufferObject->getFormat()  == GL_DEPTH_COMPONENT
        || textureBufferObject->getFormat() == GL_DEPTH_COMPONENT16
        || textureBufferObject->getFormat() == GL_DEPTH_COMPONENT24
        || textureBufferObject->getFormat() == GL_DEPTH_COMPONENT32
        || textureBufferObject->getFormat() == GL_DEPTH_COMPONENT32F)
    )
    {
        WARN("You are trying to attached a depth texture with an incorrect internal format. "
             "The depth texture will not be bound.");
        return;
    }
    
    glNamedFramebufferTexture(mFboId, GL_DEPTH_ATTACHMENT, textureBufferObject->getId(), mipLevel);
    
    validate();
}

void FramebufferObject::attachDepthBuffer(const TextureArrayObject &textureArrayObject, int layer, int mipLevel)
{
    if (!(textureArrayObject.getFormat()  == GL_DEPTH_COMPONENT
          || textureArrayObject.getFormat() == GL_DEPTH_COMPONENT16
          || textureArrayObject.getFormat() == GL_DEPTH_COMPONENT24
          || textureArrayObject.getFormat() == GL_DEPTH_COMPONENT32
          || textureArrayObject.getFormat() == GL_DEPTH_COMPONENT32F)
        )
    {
        WARN("You are trying to attached a depth texture with an incorrect internal format. "
             "The depth texture will not be bound.");
        return;
    }
    
    glNamedFramebufferTextureLayer(mFboId, GL_DEPTH_ATTACHMENT, textureArrayObject.getId(), mipLevel, layer);
    
    validate();
}

void FramebufferObject::attachDepthBuffer(const Cubemap &cubemap, int layer, int mipLevel)
{
    if (!(cubemap.getFormat()  == GL_DEPTH_COMPONENT
          || cubemap.getFormat() == GL_DEPTH_COMPONENT16
          || cubemap.getFormat() == GL_DEPTH_COMPONENT24
          || cubemap.getFormat() == GL_DEPTH_COMPONENT32
          || cubemap.getFormat() == GL_DEPTH_COMPONENT32F)
        )
    {
        WARN("You are trying to attached a depth texture with an incorrect internal format. "
             "The depth texture will not be bound.");
        return;
    }
    
    glNamedFramebufferTextureLayer(mFboId, GL_DEPTH_ATTACHMENT, cubemap.getId(), mipLevel, layer);
    
    validate();
}

void FramebufferObject::detach(int bindPoint)
{
    glNamedFramebufferTexture(mFboId, GL_COLOR_ATTACHMENT0 + bindPoint, 0, 0);
    
    const auto location = std::find(mAttachments.begin(), mAttachments.end(), GL_COLOR_ATTACHMENT0 + bindPoint);
    mAttachments.erase(location);
    
    const auto *target = mAttachments.empty() ? nullptr : &mAttachments[0];
    glNamedFramebufferDrawBuffers(mFboId, static_cast<int>(mAttachments.size()), target);
}

void FramebufferObject::validate() const
{
    const unsigned int fboStatus = glCheckNamedFramebufferStatus(mFboId, GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::string errorName;
        switch (fboStatus)
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                errorName = "framebuffer undefined";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                errorName = "framebuffer incomplete attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                errorName = "framebuffer incomplete missing attachment";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                errorName = "framebuffer incomplete drawUi buffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                errorName = "framebuffer incomplete read buffer";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                errorName = "framebuffer unsupported";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                errorName = "framebuffer incomplete multisample";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                errorName = "framebuffer incomplete layer targets";
                break;
            default:
                errorName = "Unknown";
                break;
        }
        LOG_MAJOR("Framebuffer error of: " + errorName + " (" + std::to_string(fboStatus) + ")");
    }
}

void FramebufferObject::clear(const glm::vec4 &clearColour)
{
    graphics::pushDebugGroup("Clear Pass");
    for (unsigned int attachment : mAttachments)
    {
        int drawBuffer = static_cast<int>(attachment) - GL_COLOR_ATTACHMENT0;
        glClearNamedFramebufferfv(mFboId, GL_COLOR, drawBuffer, glm::value_ptr(clearColour));
    }
    glClearNamedFramebufferfv(mFboId, GL_DEPTH, 0, &mDepthClearValue);
    graphics::popDebugGroup();
}

unsigned int FramebufferObject::getFboName() const
{
    return mFboId;
}

void FramebufferObject::bind() const
{
    glBlendFunc(mSourceBlend, mDestinationBlend);
    glDepthFunc(mDepthFunction);
    // glViewport(0, 0, mSize.x, mSize.y);  // Properly sets up the NDC for this framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, mFboId);
}


void FramebufferObject::detachRenderBuffer() const
{
    glNamedFramebufferRenderbuffer(mFboId, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
}

void FramebufferObject::detachDepthBuffer() const
{
    glNamedFramebufferTexture(mFboId, GL_DEPTH_ATTACHMENT, 0, 0);
}

void FramebufferObject::clearDepthBuffer() const
{
    glClearNamedFramebufferfv(mFboId, GL_DEPTH, 0, &mDepthClearValue);
}



