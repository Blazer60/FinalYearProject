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
        if (ui::seperatorTextButton(format::string("Masks (%/%)", mMasks.size(), glm::max(0, static_cast<int>(mLayers.size() - 1)))))
            addMask(std::make_unique<UberMask>());

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
                addNewMaterialLayer(load::materialLayer(path));
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
                out << YAML::Key << "Passthrough" << YAML::Value << static_cast<unsigned int>(mask->mPassThroughFlags);
                out << YAML::Key << "Operation" << YAML::Value << static_cast<unsigned int>(mask->mMaskOperation);
                out << YAML::EndMap;
            }
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fileOutput(mPath);
        fileOutput << out.c_str();
        fileOutput.close();

        MESSAGE_VERBOSE("Material saved to: %", mPath);
    }

    void UberMaterial::onPreRender()
    {
        if (mData.layers.size() != mLayers.size())
            ERROR("Size missmatch");

        bool anyChanges = false;
        for (int i = 0; i < mLayers.size(); ++i)
        {
            anyChanges |= !mLayers[i]->mLayerUpdates.empty();
            for (auto &action : mLayers[i]->mLayerUpdates)
                action(mTexturePool, mData.layers[i]);
        }
        for (int i = 0; i < mMasks.size(); ++i)
        {
            anyChanges |= !mMasks[i]->mMaskUpdates.empty();
            for (auto &action : mMasks[i]->mMaskUpdates)
                action(mTexturePool, mData.masks[i]);
        }

        if (anyChanges)
        {
            mData.textureArrayId = mTexturePool.id();
            mData.textureArrayData = mTexturePool.data();
        }
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
                addNewMaterialLayer(load::materialLayer(fullPath));
            }
        }

        const YAML::Node maskLayers = data["MaskLayers"];
        if (maskLayers.IsDefined() && maskLayers.IsSequence())
        {
            for (const auto &maskNode : maskLayers)
            {
                auto mask = std::make_unique<UberMask>();
                if (const YAML::Node textureNode = maskNode["Texture"]; textureNode.IsDefined())
                {
                    if (const std::string relativePath = textureNode.as<std::string>(); !relativePath.empty())
                    {
                        const std::filesystem::path fullPath = file::constructAbsolutePath(relativePath);
                        mask->mMaskTexture = load::texture(fullPath);
                    }
                }
                if (const YAML::Node alphaNode = maskNode["Alpha"]; alphaNode.IsDefined())
                {
                    const float alpha = alphaNode.as<float>();
                    mask->mAlphaThreshold = alpha;
                }
                if (const YAML::Node flagsNode = maskNode["Passthrough"]; flagsNode.IsDefined())
                    mask->mPassThroughFlags = static_cast<graphics::PassthroughFlags>(flagsNode.as<unsigned int>());
                if (const YAML::Node operationNode = maskNode["Operation"]; operationNode.IsDefined())
                    mask->mMaskOperation = static_cast<graphics::MaskOp>(operationNode.as<unsigned int>());
                addMask(std::move(mask));
            }
        }
    }

    void UberMaterial::drawMaterialLayerArray()
    {
        if (ui::seperatorTextButton(format::string("Layers (%)", mLayers.size())))
            addNewMaterialLayer(nullptr);

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
            {
                containers::moveInPlace(mLayers, *static_cast<int*>(payload->Data), index);
                containers::moveInPlace(mData.layers, *static_cast<int*>(payload->Data), index);
            }

            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceMaterialLayerPayload))
            {
                const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                addNewMaterialLayer(load::materialLayer(path), index);
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
                destroyLayer(index);
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
                destroyMask(index);
        }
        ImGui::PopID();

        return result;
    }

    void UberMaterial::updateGraphicsData()
    {
        mData.textureArrayId = mTexturePool.id();
        mData.textureArrayData = mTexturePool.data();
    }

    void UberMaterial::addMask(std::unique_ptr<UberMask> mask)
    {
        auto &maskData = mData.masks.emplace_back();
        if (mask == nullptr)
            return;

        maskData.alpha = mask->mAlphaThreshold;
        maskData.maskTextureIndex = mTexturePool.addTexture(*mask->mMaskTexture);
        maskData.passthroughFlags = static_cast<uint32_t>(mask->mPassThroughFlags);
        maskData.maskOp = static_cast<uint32_t>(mask->mMaskOperation);

        mMasks.push_back(std::move(mask));

        updateGraphicsData();
    }

    void UberMaterial::destroyMask(const int index)
    {
        const graphics::MaskData &maskData = mData.masks[index];

        mTexturePool.removeTexture(maskData.maskTextureIndex);

        mMasks.erase(mMasks.begin() + index);
        mData.masks.erase(mData.masks.begin() + index);
    }


    void UberMaterial::addNewMaterialLayer(std::shared_ptr<UberLayer> layer)
    {
        auto &layerData = mData.layers.emplace_back();
        if (layer == nullptr)
            return;

        layerData.roughness         = layer->mRoughness;
        layerData.sheenRoughness    = layer->mSheenRoughness;
        layerData.diffuseColour     = glm::vec4(layer->mDiffuseColour, 1.f);
        layerData.specularColour    = glm::vec4(layer->mSpecularColour, 1.f);
        layerData.sheenColour       = glm::vec4(layer->mSheenColour, 1.f);

        layerData.diffuseTextureIndex           = mTexturePool.addTexture(*layer->mDiffuseTexture);
        layerData.specularTextureIndex          = mTexturePool.addTexture(*layer->mSpecularTexture);
        layerData.roughnessTextureIndex         = mTexturePool.addTexture(*layer->mRoughnessTexture);
        layerData.normalTextureIndex            = mTexturePool.addTexture(*layer->mNormalTexture);
        layerData.sheenTextureIndex             = mTexturePool.addTexture(*layer->mSheenTexture);
        layerData.sheenRoughnessTextureIndex    = mTexturePool.addTexture(*layer->mSheenRoughnessTexture);

        mLayers.push_back(std::move(layer));

        updateGraphicsData();
    }

    void UberMaterial::addNewMaterialLayer(std::shared_ptr<UberLayer> layer, const int index)
    {
        graphics::LayerData &layerData = mData.layers[index];

        mTexturePool.removeTexture(layerData.diffuseTextureIndex);
        mTexturePool.removeTexture(layerData.specularTextureIndex);
        mTexturePool.removeTexture(layerData.roughnessTextureIndex);
        mTexturePool.removeTexture(layerData.normalTextureIndex);
        mTexturePool.removeTexture(layerData.sheenTextureIndex);
        mTexturePool.removeTexture(layerData.sheenRoughnessTextureIndex);

        layerData.roughness         = layer->mRoughness;
        layerData.sheenRoughness    = layer->mSheenRoughness;
        layerData.diffuseColour     = glm::vec4(layer->mDiffuseColour, 1.f);
        layerData.specularColour    = glm::vec4(layer->mSpecularColour, 1.f);
        layerData.sheenColour       = glm::vec4(layer->mSheenColour, 1.f);

        layerData.diffuseTextureIndex           = mTexturePool.addTexture(*layer->mDiffuseTexture);
        layerData.specularTextureIndex          = mTexturePool.addTexture(*layer->mSpecularTexture);
        layerData.roughnessTextureIndex         = mTexturePool.addTexture(*layer->mRoughnessTexture);
        layerData.normalTextureIndex            = mTexturePool.addTexture(*layer->mNormalTexture);
        layerData.sheenTextureIndex             = mTexturePool.addTexture(*layer->mSheenTexture);
        layerData.sheenRoughnessTextureIndex    = mTexturePool.addTexture(*layer->mSheenRoughnessTexture);

        mLayers[index] = std::move(layer);

        updateGraphicsData();
    }

    void UberMaterial::destroyLayer(const int index)
    {
        const graphics::LayerData &layer = mData.layers[index];

        mTexturePool.removeTexture(layer.diffuseTextureIndex);
        mTexturePool.removeTexture(layer.specularTextureIndex);
        mTexturePool.removeTexture(layer.roughnessTextureIndex);
        mTexturePool.removeTexture(layer.normalTextureIndex);
        mTexturePool.removeTexture(layer.sheenTextureIndex);
        mTexturePool.removeTexture(layer.sheenRoughnessTextureIndex);

        mLayers.erase(mLayers.begin() + index);
        mData.layers.erase(mData.layers.begin() + index);
    }

} // engine
