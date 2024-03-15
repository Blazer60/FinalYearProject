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

    void UberMaterial::drawMaskArray()
    {
        if (ui::seperatorTextButton(format::string("Masks (%/%)", mMasks.size(), mLayers.size() - 1)))
            mMasks.emplace_back(std::make_unique<UberMask>());

        if (ImGui::BeginTable("Mask Table", 3))
        {
            const float textSizing = ImGui::CalcTextSize("00").x;
            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, textSizing);
            ImGui::TableSetupColumn("Contents", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Delete Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, ui::buttonSize().x);

            for (int i = 0; i < mMasks.size();)
            {
                if (!drawMaskLayerElement(i))
                    ++i;
            }

            ImGui::EndTable();
        }
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

        ImGui::BeginGroup();
        drawMaskArray();
        ImGui::EndGroup();
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
        out << YAML::Key << "MaskLayers" << YAML::Value << YAML::BeginSeq;
        for (const std::unique_ptr<UberMask> &mask : mMasks)
        {
            if (mask != nullptr)
            {
                out << YAML::BeginMap;
                out << YAML::Key << "Texture" << YAML::Value << file::makeRelativeToResourcePath(mask->mMaskTexture->path()).string();
                out << YAML::Key << "Alpha" << YAML::Value << mask->mAlphaThreshold;
                out << YAML::EndMap;
            }
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fileOutput(mPath);
        fileOutput << out.c_str();
        fileOutput.close();

        MESSAGE("Material saved to: %", mPath);
    }

    void UberMaterial::onPreRender()
    {
        PROFILE_FUNC();
        graphics::pushDebugGroup(format::string("% material", mName));

        // todo: A way to not call this every frame. Could be quite slow?
        std::vector<std::shared_ptr<Texture>> validTextures;
        glm::uvec2 maxTextureSize = glm::uvec2(0);
        auto addIfValid = [&, this](const std::shared_ptr<Texture> &tex) {
            if (tex->id() > 0)
            {
                validTextures.push_back(tex);
                const glm::uvec2 size = tex->size();
                maxTextureSize = glm::max(maxTextureSize, size);
                const int index = static_cast<int>(mData.textureArrayData.size());
                mData.textureArrayData.push_back(graphics::TextureData { size.x, size.y } );
                return index;
            }

            return -1;
        };

        mData.textureArrayData.clear();
        mData.layers.resize(mLayers.size());
        for (int i = 0; i < mLayers.size(); ++i)
        {
            if (mLayers[i] == nullptr)
                continue;

            graphics::LayerData& layerData = mData.layers[i];
            const UberLayer &layer = *mLayers[i];

            layerData.roughness = layer.mRoughness;
            layerData.sheenRoughness = layer.mSheenRoughness;
            layerData.diffuseColour = glm::vec4(layer.mDiffuseColour, 1.f);
            layerData.specularColour = glm::vec4(layer.mSpecularColour, 1.f);
            layerData.sheenColour = glm::vec4(layer.mSheenColour, 1.f);

            layerData.diffuseTextureIndex = addIfValid(layer.mDiffuseTexture);
            layerData.specularTextureIndex = addIfValid(layer.mSpecularTexture);
            layerData.roughnessTextureIndex = addIfValid(layer.mRoughnessTexture);
            layerData.normalTextureIndex = addIfValid(layer.mNormalTexture);
            layerData.sheenTextureIndex = addIfValid(layer.mSheenTexture);
            layerData.sheenRoughnessTextureIndex = addIfValid(layer.mSheenRoughnessTexture);
        }

        mTextureArray.resize(maxTextureSize, static_cast<int32_t>(validTextures.size()));
        int index = 0;
        for (const std::shared_ptr<Texture> &texture : validTextures)
        {
            // todo: fucking mips. smh.
            graphics::copyTexture2D(*texture, mTextureArray, index++);
        }
        mTextureArray.setDebugName(format::string("Uber Material %", mName));
        mData.textureArrayId = mTextureArray.getId();

        graphics::popDebugGroup();
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

        const YAML::Node maskLayers = data["MaskLayers"];
        if (maskLayers.IsDefined() && maskLayers.IsSequence())
        {
            for (const auto &maskNode : maskLayers)
            {
                auto &mask = mMasks.emplace_back(std::make_unique<UberMask>());
                if (const YAML::Node textureNode = maskNode["Texture"]; textureNode.IsDefined())
                {
                    const std::string relativePath = textureNode.as<std::string>();
                    const std::filesystem::path fullPath = file::constructAbsolutePath(relativePath);
                    mask->mMaskTexture = load::texture(fullPath);
                }
                if (const YAML::Node alphaNode = maskNode["Alpha"]; alphaNode.IsDefined())
                {
                    const float alpha = alphaNode.as<float>();
                    mask->mAlphaThreshold = alpha;
                }
            }
        }
    }

    void UberMaterial::drawMaterialLayerArray()
    {
        if (ui::seperatorTextButton(format::string("Layers (%)", mLayers.size())))
            mLayers.push_back(nullptr);

        if (ImGui::BeginTable("Layers Table", 3))
        {
            const float textSizing = ImGui::CalcTextSize("00").x;
            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, textSizing);
            ImGui::TableSetupColumn("Contents", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Delete Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, ui::buttonSize().x);

            for (int i = 0; i < mLayers.size();)
            {
                if (!drawMaterialLayerElement(i))
                    ++i;
            }

            ImGui::EndTable();
        }
    }

    void UberMaterial::drawMaterialLayerElementColumn(const std::string &name, const int index)
    {
        const bool selected = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);

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

        if (selected)
        {
            ui::draw(mLayers[index]);
            ImGui::TreePop();
        }
    }

    bool UberMaterial::drawMaterialLayerElement(const int index)
    {
        const std::string name = mLayers[index] != nullptr ? mLayers[index]->name() : format::string("Empty##%", index);

        bool result = false;
        ImGui::PushID(name.c_str());
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), "%2i", index);
        }
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            drawMaterialLayerElementColumn(name, index);
        }
        if (ImGui::TableNextColumn())
        {
            result = ui::closeButton("Close innit");
            if (result)
                mLayers.erase(mLayers.begin() + index);
        }
        ImGui::PopID();

        return result;
    }

    bool UberMaterial::drawMaskLayerElement(const int index)
    {
        const std::string name = format::string("Mask %", index);

        bool result = false;
        ImGui::PushID(name.c_str());
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), "%2i", index);
        }
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                ui::draw(mMasks[index]);
                ImGui::TreePop();
            }
        }
        if (ImGui::TableNextColumn())
        {
            result = ui::closeButton("Close");
            if (result)
                mMasks.erase(mMasks.begin() + index);
        }
        ImGui::PopID();

        return result;
    }
} // engine
