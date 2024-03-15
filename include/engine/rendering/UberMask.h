/**
 * @file UberMask.h
 * @author Ryan Purse
 * @date 14/03/2024
 */


#pragma once

#include "Drawable.h"
#include "Texture.h"
#include "Pch.h"

namespace engine
{
    /**
     * A masking layer that can be used between two material layers.
     * @author Ryan Purse
     * @date 14/03/2024
     */
    class UberMask
        : public ui::Drawable
    {
        friend class UberMaterial;
    public:
        UberMask() = default;
        void onDrawUi() override;

    protected:
        std::shared_ptr<Texture> mMaskTexture = std::make_shared<Texture>("");
        float mAlphaThreshold = 0;
    };
}
