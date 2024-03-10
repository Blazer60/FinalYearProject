/**
 * @file ShaderStorageBufferObject.cpp
 * @author Ryan Purse
 * @date 24/02/2024
 */


#include "ShaderStorageBufferObject.h"

namespace graphics
{
    ShaderStorageBufferObject::ShaderStorageBufferObject(const unsigned int size, const std::string& debugName)
        : mDebugName(debugName), mSize(size)
    {
        glCreateBuffers(1, &mBufferId);

        glNamedBufferData(mBufferId, mSize, static_cast<void*>(nullptr), GL_DYNAMIC_DRAW);
        nameBuffer();
    }

    void ShaderStorageBufferObject::reserve(const unsigned int size)
    {
        if (size < mSize)
            return;

        mSize = size;
        glNamedBufferData(mBufferId, mSize, static_cast<void*>(nullptr), GL_DYNAMIC_DRAW);
        nameBuffer();
    }

    void ShaderStorageBufferObject::resize(const unsigned int size)
    {
        if (size == mSize)
            return;

        mSize = size;
        glNamedBufferData(mBufferId, mSize, static_cast<void*>(nullptr), GL_DYNAMIC_DRAW);
        nameBuffer();
    }

    void ShaderStorageBufferObject::bindToSlot(const unsigned int bindPoint)
    {
        mBindPoint = bindPoint;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, mBindPoint, mBufferId);
    }

    void ShaderStorageBufferObject::zeroOut() const
    {
        const std::vector<uint32_t> zeros(mSize);
        glNamedBufferSubData(mBufferId, 0, mSize, static_cast<const void*>(zeros.data()));
    }

    void ShaderStorageBufferObject::nameBuffer() const
    {
        if (!mDebugName.empty())
            glObjectLabel(GL_BUFFER, mBufferId, static_cast<GLsizei>(mDebugName.size()), mDebugName.c_str());
    }

}
