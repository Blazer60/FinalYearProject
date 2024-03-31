/**
 * @file UniformBufferObject.h
 * @author Ryan Purse
 * @date 05/02/2024
 */


#pragma once

#include "Pch.h"

namespace graphics
{
    template<typename TBlock>
    class UniformBufferObject
    {
    public:
        UniformBufferObject();
        ~UniformBufferObject();
        void updateGlsl() const;
        void bindToSlot(unsigned int bindPoint=0);

        TBlock *operator->() { return &mBlock; }
        const TBlock *operator->() const { return &mBlock; }
        unsigned int getBindPoint() const { return mBindPoint; };

    protected:
        unsigned int mBlockId { 0 };
        unsigned int mBindPoint { 0 };
        TBlock mBlock;
    };

    template<typename TBlock>
    UniformBufferObject<TBlock>::UniformBufferObject()
    {
        glCreateBuffers(1, &mBlockId);
        glNamedBufferStorage(mBlockId, sizeof(TBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    template<typename TBlock>
    UniformBufferObject<TBlock>::~UniformBufferObject()
    {
        if (mBlockId != 0)
            glDeleteBuffers(1, &mBlockId);
    }

    template<typename TBlock>
    void UniformBufferObject<TBlock>::updateGlsl() const
    {
        constexpr int offset = 0;
        glNamedBufferSubData(mBlockId, offset, sizeof(TBlock), static_cast<const void*>(&mBlock));
    }

    template<typename TBlock>
    void UniformBufferObject<TBlock>::bindToSlot(const unsigned int bindPoint)
    {
        constexpr int offset = 0;
        mBindPoint = bindPoint;
        glBindBufferRange(GL_UNIFORM_BUFFER, mBindPoint, mBlockId, offset, sizeof(TBlock));
    }
}
