/**
 * @file UiHelpers.cpp
 * @author Ryan Purse
 * @date 27/06/2023
 */


#include "Ui.h"

#include <imgui_internal.h>
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
    enum class buttonType : uint8_t { Cross, Plus, LeftArrow };
    bool button_impl(const ImGuiID id, const ImVec2 &pos, const buttonType type)
    {
        using namespace ImGui;
        const ImGuiContext& g = *GImGui;
        const ImGuiWindow* window = g.CurrentWindow;

        // Tweak 1: Shrink hit-testing area if button covers an abnormally large proportion of the visible region. That's in order to facilitate moving the window away. (#3825)
        // This may better be applied as a general hit-rect reduction mechanism for all widgets to ensure the area to move window is always accessible?
        const ImVec2 size = ImVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.f;
        const ImRect bb(pos, pos + size);
        ImRect bb_interact = bb;
        const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
        if (area_to_visible_ratio < 1.5f)
            bb_interact.Expand(ImFloor(bb_interact.GetSize() * -0.25f));

        // Tweak 2: We intentionally allow interaction when clipped so that a mechanical Alt,Right,Activate sequence can always close a window.
        // (this isn't the regular behavior of buttons, but it doesn't affect the user much because navigation tends to keep items visible).
        ItemSize(size);
        const bool is_clipped = !ItemAdd(bb_interact, id);

        bool hovered, held;
        const bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
        if (is_clipped)
            return pressed;

        // Render
        // FIXME: Clarify this mess
        const ImU32 col = GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
        ImVec2 center = bb.GetCenter();
        if (hovered)
            window->DrawList->AddCircleFilled(center, ImMax(2.0f, g.FontSize * 0.5f + 1.0f), col);

        const float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
        const ImU32 cross_col = GetColorU32(ImGuiCol_Text);
        center -= ImVec2(0.5f, 0.5f);
        if (type == buttonType::Plus)
        {
            window->DrawList->AddLine(center + ImVec2(+0, +cross_extent), center + ImVec2(-0, -cross_extent), cross_col, 1.0f);
            window->DrawList->AddLine(center + ImVec2(+cross_extent, -0), center + ImVec2(-cross_extent, +0), cross_col, 1.0f);
        }
        else if (type == buttonType::Cross)
        {
            window->DrawList->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), cross_col, 1.0f);
            window->DrawList->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), cross_col, 1.0f);
        }
        else if (type == buttonType::LeftArrow)
        {
            window->DrawList->AddLine(center - ImVec2(cross_extent / 2.f ,0), center + ImVec2(cross_extent, -cross_extent) - ImVec2(cross_extent / 2.f ,0), cross_col, 2.0f);
            window->DrawList->AddLine(center - ImVec2(cross_extent / 2.f ,0), center + ImVec2(cross_extent, +cross_extent) - ImVec2(cross_extent / 2.f ,0), cross_col, 2.0f);
        }

        return pressed;
    }

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
            
            ImGui::Image(reinterpret_cast<void *>(static_cast<size_t>(texture.getId())), regionSize, ImVec2(0, 1), ImVec2(1, 0));
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
            
            ImGui::Image(reinterpret_cast<void *>(static_cast<size_t>(texture.getId())), regionSize, ImVec2(0, 1), ImVec2(1, 0));
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
    
    void image(const uint32_t id, const glm::vec2 &size)
    {
        const ImVec2 imSize { size.x, size.y };
        ImGui::Image(reinterpret_cast<void *>(static_cast<size_t>(id)), imSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    
    bool imageButton(std::string_view imguiId, uint32_t glId, const glm::vec2 &size)
    {
        const ImVec2 imSize { size.x, size.y };
        return ImGui::ImageButton(imguiId.data(), reinterpret_cast<void *>(static_cast<size_t>(glId)), imSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    bool textureThumbnail(const std::string& name, std::shared_ptr<Texture>& texture)
    {
        bool result = false;
        const std::string buttonId = format::string("##%", name);
        if (ui::imageButton(buttonId, texture->id(), glm::vec2(ImGui::GetTextLineHeight())))
        {
            if (const std::string path = openFileDialog(); !path.empty())
            {
                texture = load::texture(path);
                result = true;
            }
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginTooltip())
        {
            const std::string toolTip = format::string("% Texture - %", name, texture->path().string());
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.f);
            ImGui::Text("%s", toolTip.c_str());
            ImGui::PopTextWrapPos();
            const std::string sizeText = format::string("Size: %", texture->size());
            ImGui::Text("%s", sizeText.c_str());
            ui::image(texture->id(), glm::vec3(300.f));
            ImGui::EndTooltip();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(engine::resourceImagePayload))
            {
                std::filesystem::path const path = *static_cast<std::filesystem::path*>(payload->Data);
                texture = load::texture(path);
                result = true;
            }

            ImGui::EndDragDropTarget();
        }

        return result;
    }

    float resetButtonWidth()
    {
        const auto &style = ImGui::GetStyle();
        const auto xMark = " X ";
        const float buttonWidth = ImGui::CalcTextSize(xMark, 0, true).x + style.FramePadding.x + style.ItemSpacing.x;
        return buttonWidth;
    }

    bool resetButton(const std::string& name, std::shared_ptr<Texture>& texture)
    {
        if (ui::closeButton("Delete Button"))
        {
            texture = std::make_shared<Texture>("");
            return true;
        }
        return false;
    }

    EditFlags rowTexture(const std::string& name, std::shared_ptr<Texture>& texture, graphics::WrapOp &wrapOp)
    {
        EditFlags result = EditFlags::None;

        ImGui::PushID(name.c_str());
        if (ImGui::TableNextColumn())
        {
            if (ui::textureThumbnail(name, texture))
                result = result | EditFlags::Texture;
        }
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            ImGui::Text(name.c_str());
        }
        if (ImGui::TableNextColumn())
        {

        }
        if (ImGui::TableNextColumn())
        {
            if (ui::enumCombo("##rowTextureCombo", wrapOp, graphics::wrapOpCount))
                result = result | EditFlags::Wrap;
        }
        if (ImGui::TableNextColumn())
        {
            if (resetButton(name, texture))
                result = result | EditFlags::Texture;
        }

        ImGui::PopID();
        return result;
    }

    EditFlags rowTextureColourEdit(const std::string &name, std::shared_ptr<Texture> &texture, glm::vec3 &colour, graphics::WrapOp &wrapOp)
    {
        EditFlags result = EditFlags::None;

        ImGui::PushID(name.c_str());
        if (ImGui::TableNextColumn())
        {
            if (ui::textureThumbnail(name, texture))
                result = result | EditFlags::Texture;
        }
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            ImGui::Text(name.c_str());
        }
        if (ImGui::TableNextColumn())
        {
            ui::colourEdit(format::string("##%", name), colour);
            if (ImGui::IsItemEdited())
                result = result | EditFlags::Value;
        }
        if (ImGui::TableNextColumn())
        {
            if (ui::enumCombo("##textureColourCombo", wrapOp, graphics::wrapOpCount))
                result = result | EditFlags::Wrap;
        }
        if (ImGui::TableNextColumn())
        {
            if (resetButton(name, texture))
                result = result | EditFlags::Texture;
        }

        ImGui::PopID();

        return result;
    }

    EditFlags rowTextureSliderFloat(const std::string& name, std::shared_ptr<Texture>& texture, float& value, graphics::WrapOp &wrapOp)
    {
        EditFlags result = EditFlags::None;
        ImGui::PushID(name.c_str());

        if (ImGui::TableNextColumn())
        {
            if (ui::textureThumbnail(name, texture))
                result = result | EditFlags::Texture;
        }
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            ImGui::Text(name.c_str());
        }
        if (ImGui::TableNextColumn())
        {
            const auto hidden = format::string("##Slider%", name);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SliderFloat(hidden.c_str(), &value, 0.f, 1.f);
            if (ImGui::IsItemEdited())
                result = result | EditFlags::Value;
        }
        if (ImGui::TableNextColumn())
        {
            if (ui::enumCombo("##textureSliderFloatCombo", wrapOp, graphics::wrapOpCount))
                result = result | EditFlags::Wrap;
        }
        if (ImGui::TableNextColumn())
        {
            if (resetButton(name, texture))
                result = result | EditFlags::Texture;
        }

        ImGui::PopID();
        return result;
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

    glm::ivec2 scaleImage(const glm::ivec2& imageSize, const glm::ivec2& maxSize)
    {
        const float maxSizeAspectRatio = maxSize.y / maxSize.x;
        const float imageSizeAspectRatio = imageSize.y / maxSize.x;
        return glm::vec2(imageSize) / imageSizeAspectRatio * maxSizeAspectRatio;
    }

    bool closeButton(const char *label)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const ImGuiID id = window->GetID(label);
        const ImGuiID close_button_id = ImGui::GetIDWithSeed("#CLOSE", NULL, id);
        return button_impl(close_button_id, window->DC.CursorPos, buttonType::Cross);
    }

    bool plusButton(const char* label)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const ImGuiID id = window->GetID(label);
        const ImGuiID openButtonId = ImGui::GetIDWithSeed("#OPEN", NULL, id);
        return button_impl(openButtonId, window->DC.CursorPos, buttonType::Plus);
    }

    bool backButton(const char* label)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const ImGuiID id = window->GetID(label);
        const ImGuiID openButtonId = ImGui::GetIDWithSeed("#BACK", NULL, id);
        return button_impl(openButtonId, window->DC.CursorPos, buttonType::LeftArrow);
    }

    ImVec2 buttonSize()
    {
        const ImGuiContext &g = *GImGui;
        return ImVec2(g.FontSize + 2.f * g.Style.FramePadding.x, g.FontSize + 2.f * g.Style.FramePadding.y);
    }

    bool seperatorTextButton(const std::string& name)
    {
        bool buttonClicked = false;
        const std::string tableName = format::string("SeperatorTable%", name);
        if (ImGui::BeginTable(tableName.c_str(), 2))
        {
            ImGui::TableSetupColumn("Seperator", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Adder Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, ui::buttonSize().x);

            if (ImGui::TableNextColumn())
                ImGui::SeparatorText(name.c_str());
            if (ImGui::TableNextColumn())
                buttonClicked = ui::plusButton("Add Plus Button");

            ImGui::EndTable();
        }
        return buttonClicked;
    }
}
