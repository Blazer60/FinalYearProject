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

namespace engine
{
    class Component;
}

namespace ui
{
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, bool isMainBuffer=false);
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, const glm::ivec2 &size, bool fitToRegion=true);
    void drawToolTip(std::string_view message, float tooltipWidth=20.f);
    
    bool inputText(const char *label, std::string *string, ImGuiInputTextFlags flags=0);
    
    void image(uint32_t id, const glm::vec2 &size);
    bool imageButton(std::string_view imguiId, uint32_t glId, const glm::vec2 &size);
    
    glm::ivec2 fitToRegion(const glm::ivec2 &imageSize, const glm::ivec2 &maxSize, const glm::ivec2 &padding=glm::ivec2(50));
}
