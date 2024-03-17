/**
 * @file UberMask.h
 * @author Ryan Purse
 * @date 14/03/2024
 */


#pragma once

#include "Drawable.h"
#include "Texture.h"
#include "Pch.h"
#include "TexturePool.h"

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
        typedef std::function<void(graphics::TexturePool&, graphics::MaskData&)> UpdateFunc;
    public:
        UberMask() = default;

        void drawPassthroughOptions();

        void drawOperationOptions();

        void onDrawUi() override;

    protected:
        std::shared_ptr<Texture> mMaskTexture = std::make_shared<Texture>("");
        float mAlphaThreshold = 0;
        graphics::PassthroughFlags mPassThroughFlags = graphics::PassthroughFlags::None;
        graphics::MaskOp mMaskOperation = graphics::MaskOp::Threshold;
        std::vector<UpdateFunc> mMaskUpdates;
    };
}
