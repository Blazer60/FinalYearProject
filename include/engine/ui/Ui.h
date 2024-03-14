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
    void textureThumbnail(const std::string &name, std::shared_ptr<Texture> &texture);

    float resetButtonWidth();
    void resetButton(const std::string& name, std::shared_ptr<Texture>& texture);
    void rowTexture(const std::string& name, std::shared_ptr<Texture>& texture);
    void rowTextureColourEdit(const std::string &name, std::shared_ptr<Texture> &texture, glm::vec3 &colour);
    void rowTextureSliderFloat(const std::string &name, std::shared_ptr<Texture> &texture, float &value);
    
    glm::ivec2 fitToRegion(const glm::ivec2 &imageSize, const glm::ivec2 &maxSize, const glm::ivec2 &padding=glm::ivec2(50));

    bool closeButton(const char *label);
    ImVec2 closebuttonSize();
}
