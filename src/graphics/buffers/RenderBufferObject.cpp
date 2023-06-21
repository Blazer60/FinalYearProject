/**
 * @file RenderBufferObject.cpp
 * @author Ryan Purse
 * @date 14/03/2022
 */


#include "RenderBufferObject.h"

RenderBufferObject::RenderBufferObject(const glm::ivec2 &size)
    : mSize(size)
{
    init();
}

RenderBufferObject::~RenderBufferObject()
{
    deInit();
}

void RenderBufferObject::init()
{
    glCreateRenderbuffers(1, &mId);
    glNamedRenderbufferStorage(mId, mFormat, mSize.x, mSize.y);
}

void RenderBufferObject::deInit()
{
    glDeleteRenderbuffers(1, &mId);
}

void RenderBufferObject::resize(const glm::ivec2 &size) const
{
    glNamedRenderbufferStorage(mId, mFormat, mSize.x, mSize.y);
}

GLenum RenderBufferObject::getAttachment() const
{
    return mAttachment;
}

unsigned int RenderBufferObject::getName() const
{
    return mId;
}

