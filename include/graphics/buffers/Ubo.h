/**
 * @file Ubo.h
 * @author Ryan Purse
 * @date 06/03/2024
 */


#pragma once

#include "Pch.h"

namespace graphics
{
    /**
     * Prefer Uniform Buffer Object is the type is trivially copyable because it's safer.
     * @author Ryan Purse
     * @date 06/03/2024
     */
    class Ubo
    {
    public:
        explicit Ubo(uint32_t size);
        void set(const void *data, uint32_t size=0) const;
        void bindToSlot(unsigned int bindPoint=0);
        unsigned int getBindPoint() const { return mBindPoint; }

    protected:
        unsigned int mBlockId = 0;
        unsigned int mBindPoint = 0;
        uint32_t mSize = 0;
    };

} // graphics
