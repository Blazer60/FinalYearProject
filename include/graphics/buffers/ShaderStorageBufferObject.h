/**
 * @file ShaderStorageBufferObject.h
 * @author Ryan Purse
 * @date 24/02/2024
 */


#pragma once

#include "Pch.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 24/02/2024
     */
    class ShaderStorageBufferObject
    {
    public:
        explicit ShaderStorageBufferObject(unsigned int size, const std::string &debugName="");
        void reserve(unsigned int size);
        void bindToSlot(unsigned int bindPoint);
        [[nodiscard]] unsigned int getBindPoint() const { return mBindPoint; }

        template<typename TData>
        void write(const TData &data, unsigned int offset=0);

    protected:
        void nameBuffer() const;

        std::string mDebugName;
        unsigned int mBufferId { 0 };
        unsigned int mBindPoint { 0 };
        unsigned int mSize;
    };

    template<typename TData>
    void ShaderStorageBufferObject::write(const TData &data, const unsigned int offset)
    {
        glNamedBufferSubData(mBufferId, offset, sizeof(TData), static_cast<const void*>(&data));
    }
}
