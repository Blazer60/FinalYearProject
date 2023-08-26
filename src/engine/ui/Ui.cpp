/**
 * @file UiHelpers.cpp
 * @author Ryan Purse
 * @date 27/06/2023
 */


#include "Ui.h"
#include "imgui.h"
#include "WindowHelpers.h"

namespace ui
{
    // Originally from: imgui_stdlib.cpp
    static int inputTextCallback(ImGuiInputTextCallbackData *data)
    {
        auto *string = reinterpret_cast<std::string *>(data->UserData);
        if (string == nullptr)
            LOG_MAJOR("The data passed in was not a string. It must be a string");
        else if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            if (data->Buf != string->c_str())
                LOG_MAJOR("The resizing string and buffer do not point to the same string.");
            else
            {
                string->resize(data->BufTextLen);
                data->Buf = const_cast<char *>(string->c_str());
            }
        }
        
        return 0;
    }
    
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, bool isMainBuffer)
    {
        if (show && !*show)
            return;
        
        ImGui::PushID(name.c_str());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        if (ImGui::Begin(name.c_str(), show))
        {
            ImVec2 regionSize = ImGui::GetContentRegionAvail();
            
            if (isMainBuffer)
                window::setBufferSize(glm::ivec2(regionSize.x, regionSize.y));
            
            ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    
    void showTextureBuffer(const std::string &name, const TextureBufferObject &texture, bool *show, const glm::ivec2 &size, bool fitToRegion)
    {
        if (show && !*show)
            return;
        
        ImGui::PushID(name.c_str());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        if (ImGui::Begin(name.c_str(), show))
        {
            ImVec2 regionSize = ImGui::GetContentRegionAvail();
            
            if (fitToRegion)
            {
                if (size.x == size.y)
                {
                    if (regionSize.x > regionSize.y)
                    {
                        if (static_cast<float>(size.y) >= regionSize.y)
                        {
                            const float ratio = regionSize.y / static_cast<float>(size.y);
                            regionSize.x = ratio * static_cast<float>(size.y);
                        }
                    }
                    else
                    {
                        if (static_cast<float>(size.x) >= regionSize.x)
                        {
                            const float ratio = regionSize.x / static_cast<float>(size.x);
                            regionSize.y = ratio * static_cast<float>(size.y);
                        }
                    }
                }
                if (size.x > size.y)
                {
                    if (static_cast<float>(size.x) >= regionSize.x)
                    {
                        const float ratio = regionSize.x / static_cast<float>(size.x);
                        regionSize.y = ratio * static_cast<float>(size.y);
                    }
                }
                else
                {
                    if (static_cast<float>(size.y) >= regionSize.y)
                    {
                        const float ratio = regionSize.y / static_cast<float>(size.y);
                        regionSize.x = ratio * static_cast<float>(size.y);
                    }
                }
            }
            
            ImGui::Image(reinterpret_cast<void *>(texture.getId()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopID();
    }
    
    void drawToolTip(std::string_view message, float tooltipWidth)
    {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * tooltipWidth);
            ImGui::Text("%s", message.data());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    
    bool inputText(const char *label, std::string *string, ImGuiInputTextFlags flags)
    {
        static std::string nameBeforeEdit;
        
        flags |= ImGuiInputTextFlags_CallbackResize;
        bool result = ImGui::InputText(label, const_cast<char *>(string->c_str()), string->capacity() + 1, flags, inputTextCallback, string);
        
        if (ImGui::IsItemActivated())
            nameBeforeEdit = std::string(*string);
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            if (string->empty())
            {
                WARN("The text value cannot be empty. Resetting to (%).", nameBeforeEdit);
                *string = nameBeforeEdit;
            }
        }
        
        return result;
    }
}
