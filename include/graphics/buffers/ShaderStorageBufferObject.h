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
        explicit ShaderStorageBufferObject(const std::string &debugName="");
        explicit ShaderStorageBufferObject(unsigned int size, const std::string &debugName="");
        ~ShaderStorageBufferObject();
        void reserve(unsigned int size);
        void resize(unsigned int size);
        void bindToSlot(unsigned int bindPoint);
        [[nodiscard]] unsigned int getBindPoint() const { return mBindPoint; }
        [[nodiscard]] unsigned int getId() const { return mBufferId; }

        template<typename TData>
        void write(TData *data, uint32_t size, unsigned int offset=0);

        void zeroOut() const;

    protected:
        void nameBuffer() const;

        std::string mDebugName;
        unsigned int mBufferId { 0 };
        unsigned int mBindPoint { 0 };
        unsigned int mSize;
    };

    template<typename TData>
    void ShaderStorageBufferObject::write(TData *data, const uint32_t size, const unsigned int offset)
    {
        glNamedBufferSubData(mBufferId, offset, size, static_cast<const void*>(data));
    }
}
