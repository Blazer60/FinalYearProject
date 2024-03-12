/**
 * @file UberMaterial.cpp
 * @author Ryan Purse
 * @date 11/03/2024
 */


#include "UberMaterial.h"

#include <yaml-cpp/yaml.h>

#include "ContainerAlgorithms.h"
#include "Loader.h"
#include "ResourceFolder.h"
#include "Ui.h"

namespace engine
{
    UberMaterial::UberMaterial(const std::filesystem::path& path)
        : mName(path.filename().string()), mPath(path)
    {
        if (std::filesystem::exists(mPath))
            loadFromDisk();
        else  // So that it creates an entry immediately.
            saveToDisk();
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

    void UberMaterial::saveToDisk() const
    {
        if (mPath.empty())
        {
            WARN("The material does not have a valid path! It cannot be saved.");
            return;
        }

        if (!exists(mPath.parent_path()) && !mPath.parent_path().empty())
        {
            if (!std::filesystem::create_directories(mPath.parent_path()))
            {
                WARN("Could not save material! Check that it's parent path is valid.");
                return;
            }
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "MaterialLayers" << YAML::Value << YAML::BeginSeq;
        for (const auto & materialLayer : mLayers)
        {
            if (materialLayer != nullptr)
                out << file::makeRelativeToResourcePath(materialLayer->path()).string();
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fileOutput(mPath);
        fileOutput << out.c_str();
        fileOutput.close();

        MESSAGE("Material saved to: %", mPath);
    }

    void UberMaterial::loadFromDisk()
    {
        std::ifstream stream(mPath);
        std::stringstream stringstream;
        stringstream << stream.rdbuf();
        stream.close();

        const YAML::Node data = YAML::Load(stringstream.str());
        const YAML::Node materialLayers = data["MaterialLayers"];

        if (materialLayers.IsDefined() && materialLayers.IsSequence())
        {
            for (auto layer : materialLayers)
            {
                const std::string relativePath = layer.as<std::string>();
                const std::filesystem::path fullPath = file::constructAbsolutePath(relativePath);
                mLayers.push_back(load::materialLayer(fullPath));
            }
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
        ImGui::Selectable(name.c_str(), false, 0, ImVec2(ImGui::GetContentRegionAvail().x - ui::resetButtonWidth() - 2.f * style.FramePadding.x - 2.f * style.ItemSpacing.x, 0));

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
                containers::moveInPlace(mLayers, *static_cast<int*>(payload->Data), index);

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
