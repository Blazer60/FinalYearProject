/**
 * @file FrameBufferObject.cpp
 * @author Ryan Purse
 * @date 14/03/2022
 */


#include "FramebufferObject.h"
#include "gtc/type_ptr.hpp"

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
    glNamedFramebufferTexture(mFboId, GL_COLOR_ATTACHMENT0 + bindPoint, textureBufferObject->mId, mipLevel);
    
    mAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + bindPoint);
    glNamedFramebufferDrawBuffers(mFboId, static_cast<int>(mAttachments.size()), &mAttachments[0]);
    validate();
}

void FramebufferObject::detach(int bindPoint)
{
    glNamedFramebufferTexture(mFboId, GL_COLOR_ATTACHMENT0 + bindPoint, 0, 0);
    validate();
    
    const auto location = std::find(mAttachments.begin(), mAttachments.end(), GL_COLOR_ATTACHMENT0 + bindPoint);
    mAttachments.erase(location);
    
    const auto *target = mAttachments.empty() ? nullptr : &mAttachments[0];
    glNamedFramebufferDrawBuffers(mFboId, static_cast<int>(mAttachments.size()), target);
}

void FramebufferObject::validate() const
{
    const unsigned int fboStatus = glCheckNamedFramebufferStatus(mFboId, GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        debug::log("Framebuffer error of: " + std::to_string(fboStatus), debug::severity::Major);
}

void FramebufferObject::clear(const glm::vec4 &clearColour)
{
    for (unsigned int attachment : mAttachments)
    {
        int drawBuffer = static_cast<int>(attachment) - GL_COLOR_ATTACHMENT0;
        glClearNamedFramebufferfv(mFboId, GL_COLOR, drawBuffer, glm::value_ptr(clearColour));
    }
    glClearNamedFramebufferfv(mFboId, GL_DEPTH, 0, &mDepth);
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

void FramebufferObject::attach(const RenderBufferObject *const renderBufferObject) const
{
    glNamedFramebufferRenderbuffer(mFboId, renderBufferObject->getAttachment(), GL_RENDERBUFFER,
                                   renderBufferObject->getName());
}
