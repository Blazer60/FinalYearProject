/**
 * @file UberMask.cpp
 * @author Ryan Purse
 * @date 14/03/2024
 */


#include "UberMask.h"

#include "Ui.h"

namespace engine
{
    void UberMask::onDrawUi()
    {
        mMaskUpdates.clear();

        ImGui::PushID("Masking Layer");
        if (ImGui::BeginTable("Mask Layer Table", 4))
        {
            ImGui::TableSetupColumn("Texture Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Close Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, ui::buttonSize().x);

            if (const ui::EditFlags flags = ui::rowTextureSliderFloat("Mask Alpha", mMaskTexture, mAlphaThreshold); flags > 0)
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
                    });
                }
            }

            ImGui::EndTable();
        }

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

        ImGui::PopID();
    }
}
