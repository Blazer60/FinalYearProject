/**
 * @file UberMaterial.cpp
 * @author Ryan Purse
 * @date 11/03/2024
 */


#include "UberMaterial.h"

#include "Loader.h"
#include "ResourceFolder.h"
#include "Ui.h"

namespace engine
{
    UberMaterial::UberMaterial(const std::filesystem::path& path)
        : mName(path.filename().string()), mPath(path)
    {
    }

    void UberMaterial::onDrawUi()
    {
        ImGui::Text(mName.c_str());

        ImGui::BeginGroup();
        drawMaterialLayerArray();
        ImGui::EndGroup();
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceMaterialLayerPayload))
            {
                const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                mLayers.push_back(load::materialLayer(path));
            }
            ImGui::EndDragDropTarget();
        }
    }

    void UberMaterial::drawMaterialLayerArray()
    {
        const auto name = "Material Layers";
        const auto originalCursorPos = ImGui::GetCursorPos();
        const auto PlusMarkposition = ImVec2(ImGui::GetContentRegionAvail().x - ui::resetButtonWidth(), originalCursorPos.y);

        ImGui::Text(name);
        ImGui::SetCursorPos(PlusMarkposition);

        const auto plusId = format::string(" + ##%", name);
        if (ImGui::Button(plusId.c_str()))
            mLayers.push_back(nullptr);

        for (int i = 0; i < mLayers.size();)
        {
            if (!drawMaterialLayerElement(i))
                ++i;
        }
    }

    void UberMaterial::moveElementInPlace(const int srcIndex, const int dstIndex)
    {
        if (srcIndex < dstIndex)
        {
            const auto lhs = mLayers.begin() + srcIndex;
            const auto pivot = lhs + 1;
            const auto rhs = mLayers.begin() + dstIndex + 1;
            std::rotate(lhs, pivot, rhs);
        }
        else
        {
            const auto lhs = mLayers.begin() + dstIndex;
            const auto pivot = mLayers.begin() + srcIndex;
            const auto rhs = pivot + 1;
            std::rotate(lhs, pivot, rhs);
        }
    }

    bool UberMaterial::drawMaterialLayerElement(const int index)
    {
        const std::string name = mLayers[index] != nullptr ? mLayers[index]->name() : format::string("Empty##%", index);

        auto &style = ImGui::GetStyle();
        ImGui::BeginGroup();
        const auto originalCursorPos = ImGui::GetCursorPos();
        const auto xMarkposition = ImVec2(ImGui::GetContentRegionAvail().x - ui::resetButtonWidth(), originalCursorPos.y);

        ImGui::SetCursorPosY(originalCursorPos.y + style.FramePadding.y);
        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), "%2i", index);
        ImGui::SameLine();
        ImGui::SetCursorPosY(originalCursorPos.y + style.FramePadding.y);
        ImGui::Selectable(name.c_str(), false, 0, ImVec2(ImGui::GetContentRegionAvail().x - ui::resetButtonWidth() - style.FramePadding.x - style.ItemSpacing.x, 0));

        constexpr auto arrayPayLoadId = "ARRAY_PAYLOAD";
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload(arrayPayLoadId, &index, sizeof(int));
            ImGui::Text(name.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(arrayPayLoadId))
                moveElementInPlace(*static_cast<int*>(payload->Data), index);

            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceMaterialLayerPayload))
            {
                const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                mLayers[index] = load::materialLayer(path);
            }

            ImGui::EndDragDropTarget();
        }

        const auto xMark = format::string(" X ##%", index);
        ImGui::SetCursorPos(xMarkposition);
        const bool result = ImGui::Button(xMark.c_str());
        if (result)
        {
            mLayers.erase(mLayers.begin() + index);
        }
        ImGui::EndGroup();
        return result;
    }
} // engine
