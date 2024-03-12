/**
 * @file UiHelpers.cpp
 * @author Ryan Purse
 * @date 27/06/2023
 */


#include "Ui.h"

#include <memory>

#include "Drawable.h"
#include "Editor.h"
#include "EngineState.h"
#include "FileExplorer.h"
#include "imgui.h"
#include "Loader.h"
#include "ResourceFolder.h"
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

    void colourEdit(const std::string &id, glm::vec3& colour)
    {
        ImGui::ColorEdit3(id.c_str(), glm::value_ptr(colour), colourPickerFlags);
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
    
    void image(uint32_t id, const glm::vec2 &size)
    {
        ImVec2 imSize { size.x, size.y };
        ImGui::Image(reinterpret_cast<void *>(id), imSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    
    bool imageButton(std::string_view imguiId, uint32_t glId, const glm::vec2 &size)
    {
        ImVec2 imSize { size.x, size.y };
        return ImGui::ImageButton(imguiId.data(), reinterpret_cast<void *>(glId), imSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    void textureThumbnail(const std::string& name, std::shared_ptr<Texture>& texture)
    {
        auto buttonId = format::string("##%", name);
        if (ui::imageButton(buttonId, texture->id(), glm::vec2(ImGui::GetTextLineHeight())))
        {
            engine::editor->addUpdateAction([&]() {
                const std::string result = openFileDialog();
                if (result.empty())
                    return;

                texture = load::texture(result);
            });
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginTooltip())
        {
            const std::string toolTip = format::string("% Texture - %", name, texture->path().string());
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.f);
            ImGui::Text("%s", toolTip.c_str());
            ImGui::PopTextWrapPos();
            ui::image(texture->id(), glm::vec3(300.f));
            ImGui::EndTooltip();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(engine::resourceImagePayload))
            {
                std::filesystem::path path = *reinterpret_cast<std::filesystem::path*>(payload->Data);
                engine::editor->addUpdateAction([&, path]() {
                    texture = load::texture(path);
                });
            }
        }
    }

    float resetButtonWidth()
    {
        const auto &style = ImGui::GetStyle();
        const auto xMark = " X ";
        const float buttonWidth = ImGui::CalcTextSize(xMark, 0, true).x + style.FramePadding.x + style.ItemSpacing.x;
        return buttonWidth;
    }

    void resetButton(const std::string& name, std::shared_ptr<Texture>& texture)
    {
        const auto xMark = format::string(" X ##%", name);
        const float position = ImGui::GetContentRegionAvail().x - resetButtonWidth();
        ImGui::SameLine(position);
        if (ImGui::Button(xMark.c_str()))
        {
            engine::editor->addUpdateAction([&]() {
                texture = std::make_shared<Texture>("");
            });
        }
    }

    void texture(const std::string& name, std::shared_ptr<Texture>& texture)
    {
        ImGui::PushID(name.c_str());
        auto &style = ImGui::GetStyle();
        ImGui::BeginGroup();

        ui::textureThumbnail(name, texture);

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
        ImGui::Text(name.c_str());

        resetButton(name, texture);

        ImGui::EndGroup();
        ImGui::PopID();
    }

    void textureColourEdit(const std::string& name, std::shared_ptr<Texture>& texture, glm::vec3& colour)
    {
        ImGui::PushID(name.c_str());
        auto &style = ImGui::GetStyle();
        ImGui::BeginGroup();
        const float width = ImGui::GetContentRegionAvail().x;

        ui::textureThumbnail(name, texture);

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
        ImGui::Text(name.c_str());

        ImGui::SameLine(width / 2.f);
        ui::colourEdit(format::string("##%", name), colour);

        resetButton(name, texture);

        ImGui::EndGroup();
        ImGui::PopID();
    }

    void textureSliderFloat(const std::string& name, std::shared_ptr<Texture>& texture, float& value)
    {
        ImGui::PushID(name.c_str());
        auto &style = ImGui::GetStyle();
        ImGui::BeginGroup();
        const float width = ImGui::GetContentRegionAvail().x;

        ui::textureThumbnail(name, texture);

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
        ImGui::Text(name.c_str());

        const auto hidden = format::string("##Slider%", name);
        ImGui::SameLine(width / 2.f);
        ImGui::SetNextItemWidth(width / 2.f - resetButtonWidth() - style.FramePadding.x - style.ItemSpacing.x);
        ImGui::SliderFloat(hidden.c_str(), &value, 0.f, 1.f);

        resetButton(name, texture);

        ImGui::EndGroup();
        ImGui::PopID();
    }

    glm::ivec2 fitToRegion(const glm::ivec2 &imageSize, const glm::ivec2 &maxSize, const glm::ivec2 &padding)
    {
        ImVec2 regionSize = ImGui::GetContentRegionAvail();
        regionSize.x -= static_cast<float>(padding.x);
        regionSize.y -= static_cast<float>(padding.y);
        regionSize.x = glm::min(regionSize.x, static_cast<float>(maxSize.x));
        regionSize.y = glm::min(regionSize.y, static_cast<float>(maxSize.y));
        
        if (imageSize.x <= 0 || imageSize.y <= 0)
            return { regionSize.x, regionSize.y };
        
        if (imageSize.x == imageSize.y)
        {
            if (regionSize.x > regionSize.y)
            {
                if (static_cast<float>(imageSize.y) >= regionSize.y)
                {
                    const float ratio = regionSize.y / static_cast<float>(imageSize.y);
                    regionSize.x = ratio * static_cast<float>(imageSize.y);
                }
            }
            else
            {
                if (static_cast<float>(imageSize.x) >= regionSize.x)
                {
                    const float ratio = regionSize.x / static_cast<float>(imageSize.x);
                    regionSize.y = ratio * static_cast<float>(imageSize.y);
                }
            }
        }
        if (imageSize.x > imageSize.y)
        {
            if (static_cast<float>(imageSize.x) >= regionSize.x)
            {
                const float ratio = regionSize.x / static_cast<float>(imageSize.x);
                regionSize.y = ratio * static_cast<float>(imageSize.y);
            }
        }
        else
        {
            if (static_cast<float>(imageSize.y) >= regionSize.y)
            {
                const float ratio = regionSize.y / static_cast<float>(imageSize.y);
                regionSize.x = ratio * static_cast<float>(imageSize.y);
            }
        }
        
        return { regionSize.x, regionSize.y };
    }
}
