/**
 * @file Ubo.cpp
 * @author Ryan Purse
 * @date 06/03/2024
 */


#include "Ubo.h"

#include "Logger.h"
#include "LoggerMacros.h"

namespace graphics
{
    Ubo::Ubo(const uint32_t size)
        : mSize(size)
    {
        glCreateBuffers(1, &mBlockId);
        glNamedBufferStorage(mBlockId, mSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    void Ubo::set(const void* data, uint32_t size) const
    {
        if (size == 0)
            size = mSize;
        else if (size > mSize)
        {
            WARN("Trying to fill a buffer with size % but there's only enough space for %", size, mSize);
            size = mSize;
        }

        constexpr int offset = 0;
        glNamedBufferSubData(mBlockId, offset, size, data);
    }

    void Ubo::bindToSlot(const unsigned int bindPoint)
    {
        constexpr int offset = 0;
        mBindPoint = bindPoint;
        glBindBufferRange(GL_UNIFORM_BUFFER, mBindPoint, mBlockId, offset, mSize);
    }
}
