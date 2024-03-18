/**
 * @file Ui.h
 * @author Ryan Purse
 * @date 27/06/2023
 */


#pragma once

#include "Pch.h"
#include "TextureBufferObject.h"
#include "TextureArrayObject.h"
#include "imgui.h"
#include "MaterialData.h"
#include "Texture.h"

namespace engine
{
    class Component;
}

namespace ui
{
    constexpr int colourPickerFlags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel;

    void colourEdit(const std::string &id, glm::vec3 &colour);
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, bool isMainBuffer=false);
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, const glm::ivec2 &size, bool fitToRegion=true);
    void drawToolTip(std::string_view message, float tooltipWidth=20.f);
    
    bool inputText(const char *label, std::string *string, ImGuiInputTextFlags flags=0);
    
    void image(uint32_t id, const glm::vec2 &size);
    bool imageButton(std::string_view imguiId, uint32_t glId, const glm::vec2 &size);
    bool textureThumbnail(const std::string &name, std::shared_ptr<Texture> &texture);

    enum class EditFlags : uint8_t
    {
        None        = 0,
        Value       = 1 << 0,
        Texture     = 1 << 1,
        Wrap        = 1 << 2,
    };

    inline EditFlags operator|(EditFlags lhs, EditFlags rhs)
    {
        return static_cast<EditFlags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    inline int operator&(EditFlags lhs, EditFlags rhs)
    {
        return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
    }

    inline bool operator>(EditFlags lhs, const int rhs)
    {
        return static_cast<int>(lhs) > rhs;
    }


    float resetButtonWidth();
    bool resetButton(const std::string& name, std::shared_ptr<Texture>& texture);
    EditFlags rowTexture(const std::string& name, std::shared_ptr<Texture>& texture, graphics::WrapOp &wrapOp);
    EditFlags rowTextureColourEdit(const std::string &name, std::shared_ptr<Texture> &texture, glm::vec3 &colour, graphics::WrapOp &wrapOp);
    EditFlags rowTextureSliderFloat(const std::string &name, std::shared_ptr<Texture> &texture, float &value, graphics::WrapOp &wrapOp);
    
    glm::ivec2 fitToRegion(const glm::ivec2 &imageSize, const glm::ivec2 &maxSize, const glm::ivec2 &padding=glm::ivec2(50));

    bool closeButton(const char *label);
    bool plusButton(const char *label);
    bool backButton(const char *label);
    ImVec2 buttonSize();

    bool seperatorTextButton(const std::string &name);

    template<typename TEnum>
    bool enumCombo(const std::string &name, TEnum &value, uint32_t enumCount);

}

namespace ui
{
    template<typename TEnum>
    bool enumCombo(const std::string &name, TEnum& value, const uint32_t enumCount)
    {
        bool hasChanged = false;
        auto &style = ImGui::GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 12);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12);
        ImGui::SetNextItemWidth(ImGui::CalcTextSize(to_string(value)).x + 2.f * style.ItemInnerSpacing.x);
        if (ImGui::BeginCombo(name.c_str(), to_string(value), ImGuiComboFlags_NoArrowButton))
        {
            for (uint32_t i = 0; i < enumCount; ++i)
            {
                if (ImGui::Selectable(to_string(static_cast<TEnum>(i))))
                {
                    value = static_cast<TEnum>(i);
                    hasChanged = true;
                }
            }

            ImGui::EndCombo();
        }
        ImGui::PopStyleVar(2);

        return hasChanged;
    }
}
