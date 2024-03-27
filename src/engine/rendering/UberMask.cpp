/**
 * @file UberMask.cpp
 * @author Ryan Purse
 * @date 14/03/2024
 */


#include "UberMask.h"

#include "Ui.h"

namespace engine
{
    UberMask::UberMask()
    {
        mCallbackToken = engine::resourcePool->onTextureReady.subscribe([this](const std::shared_ptr<Texture> &texture) {
            if (mMaskTexture == texture)
            {
                mMaskUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::MaskData &mask) {
                    texturePool.removeTexture(mask.maskTextureIndex);
                    mask.maskTextureIndex = texturePool.addTexture(*mMaskTexture);
                    mask.maskOp = static_cast<uint32_t>(mMaskOperation);
                });
            }
        });
    }

    UberMask::~UberMask()
    {
        engine::resourcePool->onTextureReady.unSubscribe(mCallbackToken);
    }

    void UberMask::drawPassthroughOptions()
    {
        ImGui::SeparatorText("Parameter passthrough");
        auto flagCheckbox = [](graphics::PassthroughFlags &flags, graphics::PassthroughFlags flag) {
            const std::string name = graphics::to_string(flag);
            return ImGui::CheckboxFlags(name.c_str(), reinterpret_cast<unsigned int*>(&flags), static_cast<unsigned int>(flag));
        };

        bool hasFlagUpdated = false;
        for (int i = 0; i < graphics::passthroughFlagCount; ++i)
            hasFlagUpdated |= flagCheckbox(mPassThroughFlags, static_cast<graphics::PassthroughFlags>(1 << i));

        if (hasFlagUpdated)
        {
            mMaskUpdates.push_back([this](graphics::TexturePool &, graphics::MaskData &mask) {
                mask.passthroughFlags = static_cast<uint32_t>(mPassThroughFlags);
            });
        }
    }

    void UberMask::drawOperationOptions()
    {
        auto radio = [](graphics::MaskOp &operation, const graphics::MaskOp maskOp) {
            const std::string name = graphics::to_string(maskOp);
            const bool editied = ImGui::RadioButton(name.c_str(), operation == maskOp);
            if (editied)
                operation = maskOp;
            return editied;
        };

        bool hasChanged = false;
        for (int i = 0; i < graphics::maskOpCount; ++i)
        {
            if (i != 0)
                ImGui::SameLine();
            hasChanged |= radio(mMaskOperation, static_cast<graphics::MaskOp>(i));
        }

        if (hasChanged)
        {
            mMaskUpdates.push_back([this](graphics::TexturePool &, graphics::MaskData &mask) {
                mask.maskOp = static_cast<uint32_t>(mMaskOperation);
            });
        }
    }

    void UberMask::onDrawUi()
    {
        mMaskUpdates.clear();  // todo: Masks should be part of the resource pool so that they can be reused and cleaned.
        // todo: Make models load async. Should be able to load sponza then.

        ImGui::PushID("Masking Layer");
        if (ImGui::BeginTable("Mask Layer Table", 5))
        {
            ImGui::TableSetupColumn("Texture Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Wrap Operation", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Close Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);

            if (const ui::EditFlags flags = ui::rowTextureSliderFloat("Mask Alpha", mMaskTexture, mAlphaThreshold, mWrapOperation); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    mMaskUpdates.push_back([this](graphics::TexturePool &, graphics::MaskData &mask) {
                        mask.alpha = mAlphaThreshold;
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    mMaskUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::MaskData &mask) {
                        texturePool.removeTexture(mask.maskTextureIndex);
                        mask.maskTextureIndex = texturePool.addTexture(*mMaskTexture);
                        mask.maskOp = static_cast<uint32_t>(mMaskOperation);
                    });
                }
            }

            ImGui::EndTable();
        }

        drawOperationOptions();
        drawPassthroughOptions();

        ImGui::PopID();
    }
}
