/**
 * @file UberLayer.cpp
 * @author Ryan Purse
 * @date 11/03/2024
 */


#include "UberLayer.h"

#include "Editor.h"
#include "EngineState.h"
#include "FileExplorer.h"
#include "Loader.h"
#include "Ui.h"

namespace engine
{
    UberLayer::UberLayer(const std::filesystem::path& path)
        : mName(path.filename().string()), mPath(path)
    {
        mCallbackToken = engine::resourcePool->onTextureReady.subscribe([this](const std::shared_ptr<Texture> &texture) {
            lookForTextureChange(texture);
        });

        if (std::filesystem::exists(mPath))
            loadFromDisk();
        else  // So that it creates an entry immediately.
            saveToDisk();
    }

    UberLayer::~UberLayer()
    {
        engine::resourcePool->onTextureReady.unSubscribe(mCallbackToken);
    }

    void UberLayer::onDrawUi()
    {
        if (ImGui::BeginTable("Default Settings Table", 5))
        {
            ImGui::TableSetupColumn("Texture Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Slider Or Value", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Wrap Operation", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Close Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);

            if (const ui::EditFlags flags = ui::rowTextureColourEdit("diffuse", mDiffuseTexture, mDiffuseColour, mDiffuseWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.diffuseColour = glm::vec4(mDiffuseColour, 1.f);
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.diffuseTextureIndex);
                        layer.diffuseTextureIndex = texturePool.addTexture(*mDiffuseTexture);
                        texturePool.setWrap(layer.diffuseTextureIndex, mDiffuseWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.diffuseTextureIndex, mDiffuseWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTextureColourEdit("specular", mSpecularTexture, mSpecularColour, mSpecularWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.specularColour = glm::vec4(mSpecularColour, 1.f);
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.specularTextureIndex);
                        layer.specularTextureIndex = texturePool.addTexture(*mSpecularTexture);
                        texturePool.setWrap(layer.specularTextureIndex, mSpecularWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.specularTextureIndex, mSpecularWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTexture("normal map", mNormalTexture, mNormalWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.normalTextureIndex);
                        layer.normalTextureIndex = texturePool.addTexture(*mNormalTexture);
                        texturePool.setWrap(layer.normalTextureIndex, mNormalWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.normalTextureIndex, mNormalWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTextureSliderFloat("roughness", mRoughnessTexture, mRoughness, mRoughnessWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.roughness = mRoughness;
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.roughnessTextureIndex);
                        layer.roughnessTextureIndex = texturePool.addTexture(*mRoughnessTexture);
                        texturePool.setWrap(layer.roughnessTextureIndex, mRoughnessWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.roughnessTextureIndex, mRoughnessWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTextureColourEdit("Sheen Colour", mSheenTexture, mSheenColour, mSheenWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.sheenColour = glm::vec4(mSheenColour, 1.f);
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.sheenTextureIndex);
                        layer.sheenTextureIndex = texturePool.addTexture(*mSheenTexture);
                        texturePool.setWrap(layer.sheenTextureIndex, mSheenRoughnessWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.sheenTextureIndex, mSheenRoughnessWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTextureSliderFloat("Sheen Roughness", mSheenRoughnessTexture, mSheenRoughness, mSheenRoughnessWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.sheenRoughness = mSheenRoughness;
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.sheenRoughnessTextureIndex);
                        layer.sheenRoughnessTextureIndex = texturePool.addTexture(*mSheenRoughnessTexture);
                        texturePool.setWrap(layer.sheenRoughnessTextureIndex, mSheenRoughnessWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.sheenRoughnessTextureIndex, mSheenRoughnessWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTexture("Metallic map", mMetallicTexture, mMetallicWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.metallicTextureIndex);
                        layer.metallicTextureIndex = texturePool.addTexture(*mMetallicTexture);
                        texturePool.setWrap(layer.metallicTextureIndex, mMetallicWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.metallicTextureIndex, mMetallicWrapOp);
                    });
                }
            }

            if (const ui::EditFlags flags = ui::rowTextureColourEdit("Top Specular Map", mTopSpecularTexture, mTopSpecularColour, mTopSpecularWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.topSpecularColour = glm::vec4(mTopSpecularColour, 1.f);
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.topSpecularColourTextureIndex);
                        layer.topSpecularColourTextureIndex = texturePool.addTexture(*mTopSpecularTexture);
                        texturePool.setWrap(layer.topSpecularColourTextureIndex, mTopSpecularWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.topSpecularColourTextureIndex, mTopSpecularWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTexture("Top Normal Map", mTopNormalTexture, mTopNormalWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.topNormalTextureIndex);
                        layer.topNormalTextureIndex = texturePool.addTexture(*mTopNormalTexture);
                        texturePool.setWrap(layer.topNormalTextureIndex, mTopNormalWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.topNormalTextureIndex, mTopNormalWrapOp);
                    });
                }
            }
            if (const ui::EditFlags flags = ui::rowTextureColourEdit("Transmittance", mTransmittanceTexture, mTransmittanceColour, mTransmittanceWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.transmittanceColour = glm::vec4(mTransmittanceColour, 1.f);
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.transmittanceColourTextureIndex);
                        layer.transmittanceColourTextureIndex = texturePool.addTexture(*mTransmittanceTexture);
                        texturePool.setWrap(layer.transmittanceColourTextureIndex, mTransmittanceWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.transmittanceColourTextureIndex, mTransmittanceWrapOp);
                    });
                }
            }

            if (const ui::EditFlags flags = ui::rowTextureSliderFloat("Top Roughness", mTopRoughnessTexture, mTopRoughness, mTopRoughnessWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.topRoughness = mTopRoughness;
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.topRoughnessTextureIndex);
                        layer.topRoughnessTextureIndex = texturePool.addTexture(*mTopRoughnessTexture);
                        texturePool.setWrap(layer.topRoughnessTextureIndex, mTopRoughnessWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.topRoughnessTextureIndex, mTopRoughnessWrapOp);
                    });
                }
            }

            if (const ui::EditFlags flags = ui::rowTextureSliderFloat("Thickness", mTopThicknessTexture, mTopThickness, mTopThicknessWrapOp, minThickness, maxThickness); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.topThickness = remapThickness(mTopThickness);
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.topThicknessTextureIndex);
                        layer.topThicknessTextureIndex = texturePool.addTexture(*mTopThicknessTexture);
                        texturePool.setWrap(layer.topThicknessTextureIndex, mTopThicknessWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.topThicknessTextureIndex, mTopThicknessWrapOp);
                    });
                }
            }

            if (const ui::EditFlags flags = ui::rowTextureSliderFloat("Coverage", mTopCoverageTexture, mTopCoverage, mTopCoverageWrapOp); flags > 0)
            {
                if ((flags & ui::EditFlags::Value) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &, graphics::LayerData &layer) {
                        layer.topCoverage = mTopCoverage;
                    });
                }
                if ((flags & ui::EditFlags::Texture) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                        texturePool.removeTexture(layer.topCoverageTextureIndex);
                        layer.topCoverageTextureIndex = texturePool.addTexture(*mTopCoverageTexture);
                        texturePool.setWrap(layer.topCoverageTextureIndex, mTopCoverageWrapOp);
                    });
                }
                if ((flags & ui::EditFlags::Wrap) > 0)
                {
                    layerUpdates.push_back([this](graphics::TexturePool &texturePool, const graphics::LayerData &layer) {
                        texturePool.setWrap(layer.topCoverageTextureIndex, mTopCoverageWrapOp);
                    });
                }
            }

            ImGui::EndTable();
        }
        const auto name = format::string("%", mPath);
        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), name.c_str());
        ImGui::PopTextWrapPos();
    }

    void UberLayer::saveToDisk() const
    {
        if (mPath.empty())
        {
            WARN("The material layer does not have a valid path! It cannont be saved.");
            return;
        }

        if (!exists(mPath.parent_path()) && !mPath.parent_path().empty())
        {
            if (!std::filesystem::create_directories(mPath.parent_path()))
            {
                WARN("Could not save material layer! Check that it's parent path is valid.");
                return;
            }
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "DiffuseTexturePath"        << YAML::Value << file::makeRelativeToResourcePath(mDiffuseTexture->path()).string();
        out << YAML::Key << "SpecularTexturePath"       << YAML::Value << file::makeRelativeToResourcePath(mSpecularTexture->path()).string();
        out << YAML::Key << "NormalTexturePath"         << YAML::Value << file::makeRelativeToResourcePath(mNormalTexture->path()).string();
        out << YAML::Key << "RoughnessTexturePath"      << YAML::Value << file::makeRelativeToResourcePath(mRoughnessTexture->path()).string();
        out << YAML::Key << "SheenTexturePath"          << YAML::Value << file::makeRelativeToResourcePath(mSheenTexture->path()).string();
        out << YAML::Key << "SheenRoughnessTexturePath" << YAML::Value << file::makeRelativeToResourcePath(mSheenRoughnessTexture->path()).string();
        out << YAML::Key << "MetallicTexturePath"       << YAML::Value << file::makeRelativeToResourcePath(mMetallicTexture->path()).string();
        out << YAML::Key << "TopSpecularTexturePath"    << YAML::Value << file::makeRelativeToResourcePath(mTopSpecularTexture->path()).string();
        out << YAML::Key << "TransmittanceTexturePath"  << YAML::Value << file::makeRelativeToResourcePath(mTransmittanceTexture->path()).string();
        out << YAML::Key << "TopRoughnessTexturePath"   << YAML::Value << file::makeRelativeToResourcePath(mTopRoughnessTexture->path()).string();
        out << YAML::Key << "TopThicknessTexturePath"   << YAML::Value << file::makeRelativeToResourcePath(mTopThicknessTexture->path()).string();
        out << YAML::Key << "TopCoverageTexturePath"    << YAML::Value << file::makeRelativeToResourcePath(mTopCoverageTexture->path()).string();
        out << YAML::Key << "TopNormalTexturePath"      << YAML::Value << file::makeRelativeToResourcePath(mTopNormalTexture->path()).string();

        out << YAML::Key << "DiffuseColour"         << YAML::Value << mDiffuseColour;
        out << YAML::Key << "SpecularColour"        << YAML::Value << mSpecularColour;
        out << YAML::Key << "Roughness"             << YAML::Value << mRoughness;
        out << YAML::Key << "SheenColour"           << YAML::Value << mSheenColour;
        out << YAML::Key << "SheenRoughness"        << YAML::Value << mSheenRoughness;
        out << YAML::Key << "TopSpecularColour"     << YAML::Value << mTopSpecularColour;
        out << YAML::Key << "TopRoughness"          << YAML::Value << mTopRoughness;
        out << YAML::Key << "TransmittanceColour"   << YAML::Value << mTransmittanceColour;
        out << YAML::Key << "TopThickness"          << YAML::Value << mTopThickness;
        out << YAML::Key << "TopCoverage"           << YAML::Value << mTopCoverage;

        out << YAML::Key << "DiffuseWrapOp"         << YAML::Value << static_cast<unsigned int>(mDiffuseWrapOp);
        out << YAML::Key << "SpecularWrapOp"        << YAML::Value << static_cast<unsigned int>(mSpecularWrapOp);
        out << YAML::Key << "NormalWrapOp"          << YAML::Value << static_cast<unsigned int>(mNormalWrapOp);
        out << YAML::Key << "RoughnessWrapOp"       << YAML::Value << static_cast<unsigned int>(mRoughnessWrapOp);
        out << YAML::Key << "SheenWrapOp"           << YAML::Value << static_cast<unsigned int>(mSheenWrapOp);
        out << YAML::Key << "SheenRoughnessWrapOp"  << YAML::Value << static_cast<unsigned int>(mSheenRoughnessWrapOp);
        out << YAML::Key << "TopSpecularWrapOp"     << YAML::Value << static_cast<unsigned int>(mTopSpecularWrapOp);
        out << YAML::Key << "TransmittanceWrapOp"   << YAML::Value << static_cast<unsigned int>(mTransmittanceWrapOp);
        out << YAML::Key << "TopRoughnessWrapOp"    << YAML::Value << static_cast<unsigned int>(mTopRoughnessWrapOp);
        out << YAML::Key << "TopThicknessWrapOp"    << YAML::Value << static_cast<unsigned int>(mTopThicknessWrapOp);
        out << YAML::Key << "TopCoverageWrapOp"     << YAML::Value << static_cast<unsigned int>(mTopCoverageWrapOp);
        out << YAML::EndMap;

        std::ofstream fileOutput(mPath);
        fileOutput << out.c_str();
        fileOutput.close();

        MESSAGE_VERBOSE("Material Layer save to: %", mPath);
    }

    void UberLayer::lookForTextureChange(const std::shared_ptr<Texture>& texture)
    {
        if (mDiffuseTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.diffuseTextureIndex);
                layer.diffuseTextureIndex = texturePool.addTexture(*mDiffuseTexture);
                texturePool.setWrap(layer.diffuseTextureIndex, mDiffuseWrapOp);
            });
        }
        else if (mSpecularTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.specularTextureIndex);
                layer.specularTextureIndex = texturePool.addTexture(*mSpecularTexture);
                texturePool.setWrap(layer.specularTextureIndex, mSpecularWrapOp);
            });
        }
        else if (mNormalTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.normalTextureIndex);
                layer.normalTextureIndex = texturePool.addTexture(*mNormalTexture);
                texturePool.setWrap(layer.normalTextureIndex, mNormalWrapOp);
            });
        }
        else if (mRoughnessTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.roughnessTextureIndex);
                layer.roughnessTextureIndex = texturePool.addTexture(*mRoughnessTexture);
                texturePool.setWrap(layer.roughnessTextureIndex, mRoughnessWrapOp);
            });
        }
        else if (mSheenTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.sheenTextureIndex);
                layer.sheenTextureIndex = texturePool.addTexture(*mSheenTexture);
                texturePool.setWrap(layer.sheenTextureIndex, mSheenWrapOp);
            });
        }
        else if (mSheenRoughnessTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.sheenRoughnessTextureIndex);
                layer.sheenRoughnessTextureIndex = texturePool.addTexture(*mSheenRoughnessTexture);
                texturePool.setWrap(layer.sheenRoughnessTextureIndex, mSheenRoughnessWrapOp);
            });
        }
        else if (mMetallicTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.metallicTextureIndex);
                layer.metallicTextureIndex = texturePool.addTexture(*mMetallicTexture);
                texturePool.setWrap(layer.metallicTextureIndex, mMetallicWrapOp);
            });
        }

        else if (mTopSpecularTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.topSpecularColourTextureIndex);
                layer.topSpecularColourTextureIndex = texturePool.addTexture(*mTopSpecularTexture);
                texturePool.setWrap(layer.topSpecularColourTextureIndex, mTopSpecularWrapOp);
            });
        }
        else if (mTransmittanceTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.transmittanceColourTextureIndex);
                layer.transmittanceColourTextureIndex = texturePool.addTexture(*mTransmittanceTexture);
                texturePool.setWrap(layer.transmittanceColourTextureIndex, mTransmittanceWrapOp);
            });
        }
        else if (mTopRoughnessTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.topRoughnessTextureIndex);
                layer.topRoughnessTextureIndex = texturePool.addTexture(*mTopRoughnessTexture);
                texturePool.setWrap(layer.topRoughnessTextureIndex, mTopRoughnessWrapOp);
            });
        }
        else if (mTopThicknessTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.topThicknessTextureIndex);
                layer.topThicknessTextureIndex = texturePool.addTexture(*mTopThicknessTexture);
                texturePool.setWrap(layer.topThicknessTextureIndex, mTopThicknessWrapOp);
            });
        }
        else if (mTopCoverageTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.topCoverageTextureIndex);
                layer.topCoverageTextureIndex = texturePool.addTexture(*mTopCoverageTexture);
                texturePool.setWrap(layer.topCoverageTextureIndex, mTopCoverageWrapOp);
            });
        }
        else if (mTopNormalTexture == texture)
        {
            layerUpdates.push_back([this](graphics::TexturePool &texturePool, graphics::LayerData &layer) {
                texturePool.removeTexture(layer.topNormalTextureIndex);
                layer.topNormalTextureIndex = texturePool.addTexture(*mTopNormalTexture);
                texturePool.setWrap(layer.topNormalTextureIndex, mTopNormalWrapOp);
            });
        }
    }

    float UberLayer::remapThickness(const float topThickness)
    {
        return math::inverseLerp(minThickness, maxThickness, topThickness);
    }

    void UberLayer::loadFromDisk()
    {
        std::ifstream stream(mPath);
        std::stringstream stringstream;
        stringstream << stream.rdbuf();
        stream.close();

        auto tryLoadAsTexture = [](const YAML::Node &node) {
            if (node.IsDefined())
            {
                if (const std::string relativePath = node.as<std::string>(); !relativePath.empty())
                    return load::texture(file::constructAbsolutePath(relativePath));
            }
            return load::texture("");
        };

        auto tryLoadAsVec3 = [](const YAML::Node &node, glm::vec3 &v) {
            if (node.IsDefined())
                v = node.as<glm::vec3>();
        };

        auto tryLoadAsFloat = [](const YAML::Node &node, float &f) {
            if (node.IsDefined())
                f = node.as<float>();
        };

        auto tryLoadAsWrapOp = [](const YAML::Node &node, graphics::WrapOp &wrapOp) {
            if (node.IsDefined())
                wrapOp = static_cast<graphics::WrapOp>(node.as<unsigned int>());
        };

        YAML::Node data = YAML::Load(stringstream.str());
        mDiffuseTexture             = tryLoadAsTexture(data["DiffuseTexturePath"]);
        mSpecularTexture            = tryLoadAsTexture(data["SpecularTexturePath"]);
        mNormalTexture              = tryLoadAsTexture(data["NormalTexturePath"]);
        mRoughnessTexture           = tryLoadAsTexture(data["RoughnessTexturePath"]);
        mSheenTexture               = tryLoadAsTexture(data["SheenTexturePath"]);
        mSheenRoughnessTexture      = tryLoadAsTexture(data["SheenRoughnessTexturePath"]);
        mMetallicTexture            = tryLoadAsTexture(data["MetallicTexturePath"]);
        mTopSpecularTexture         = tryLoadAsTexture(data["TopSpecularTexturePath"]);
        mTransmittanceTexture       = tryLoadAsTexture(data["TransmittanceTexturePath"]);
        mTopRoughnessTexture        = tryLoadAsTexture(data["TopRoughnessTexturePath"]);
        mTopThicknessTexture        = tryLoadAsTexture(data["TopThicknessTexturePath"]);
        mTopCoverageTexture         = tryLoadAsTexture(data["TopCoverageTexturePath"]);
        mTopNormalTexture           = tryLoadAsTexture(data["TopNormalTexturePath"]);

        tryLoadAsVec3( data["DiffuseColour"],       mDiffuseColour);
        tryLoadAsVec3( data["SpecularColour"],      mSpecularColour);
        tryLoadAsFloat(data["Roughness"],           mRoughness);
        tryLoadAsVec3( data["SheenColour"],         mSheenColour);
        tryLoadAsFloat(data["SheenRoughness"],      mSheenRoughness);
        tryLoadAsVec3( data["TopSpecularColour"],   mTopSpecularColour);
        tryLoadAsFloat(data["TopRoughness"],        mTopRoughness);
        tryLoadAsVec3( data["TransmittanceColour"], mTransmittanceColour);
        tryLoadAsFloat(data["TopThickness"],        mTopThickness);
        tryLoadAsFloat(data["TopCoverage"],         mTopCoverage);

        tryLoadAsWrapOp(data["DiffuseWrapOp"],          mDiffuseWrapOp);
        tryLoadAsWrapOp(data["SpecularWrapOp"],         mSpecularWrapOp);
        tryLoadAsWrapOp(data["NormalWrapOp"],           mNormalWrapOp);
        tryLoadAsWrapOp(data["RoughnessWrapOp"],        mRoughnessWrapOp);
        tryLoadAsWrapOp(data["SheenWrapOp"],            mSheenWrapOp);
        tryLoadAsWrapOp(data["SheenRoughnessWrapOp"],   mSheenRoughnessWrapOp);
        tryLoadAsWrapOp(data["MetallicWrapOp"],         mMetallicWrapOp);
        tryLoadAsWrapOp(data["TopSpecularWrapOp"],      mTopSpecularWrapOp);
        tryLoadAsWrapOp(data["TransmittanceWrapOp"],    mTransmittanceWrapOp);
        tryLoadAsWrapOp(data["TopRoughnessWrapOp"],     mTopRoughnessWrapOp);
        tryLoadAsWrapOp(data["TopThicknessWrapOp"],     mTopThicknessWrapOp);
        tryLoadAsWrapOp(data["TopCoverageWrapOp"],      mTopCoverageWrapOp);
    }
} // engine
