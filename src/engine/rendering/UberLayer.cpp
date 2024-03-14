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
        if (std::filesystem::exists(mPath))
            loadFromDisk();
        else  // So that it creates an entry immediately.
            saveToDisk();
    }

    void UberLayer::onDrawUi()
    {
        if (ImGui::BeginTable("Default Settings Table", 4))
        {
            ImGui::TableSetupColumn("Texture Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Slider Or Value", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Close Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, ui::buttonSize().x);

            ui::rowTextureColourEdit("diffuse", mDiffuseTexture, mDiffuseColour);
            ui::rowTextureColourEdit("specular", mSpecularTexture, mSpecularColour);
            ui::rowTexture("normal map", mNormalTexture);
            ui::rowTextureSliderFloat("roughness", mRoughnessTexture, mRoughness);
            ui::rowTextureColourEdit("Sheen Colour", mSheenTexture, mSheenColour);
            ui::rowTextureSliderFloat("Sheen Roughness", mSheenRoughnessTexture, mSheenRoughness);
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

        out << YAML::Key << "DiffuseColour"     << YAML::Value << mDiffuseColour;
        out << YAML::Key << "SpecularColour"    << YAML::Value << mSpecularColour;
        out << YAML::Key << "Roughness"         << YAML::Value << mRoughness;
        out << YAML::Key << "SheenColour"       << YAML::Value << mSheenColour;
        out << YAML::Key << "SheenRoughness"    << YAML::Value << mSheenRoughness;
        out << YAML::EndMap;

        std::ofstream fileOutput(mPath);
        fileOutput << out.c_str();
        fileOutput.close();

        MESSAGE("Material Layer save to: %", mPath);
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

        YAML::Node data = YAML::Load(stringstream.str());
        mDiffuseTexture         = tryLoadAsTexture(data["DiffuseTexturePath"]);
        mSpecularTexture        = tryLoadAsTexture(data["SpecularTexturePath"]);
        mNormalTexture          = tryLoadAsTexture(data["NormalTexturePath"]);
        mRoughnessTexture       = tryLoadAsTexture(data["RoughnessTexturePath"]);
        mSheenTexture           = tryLoadAsTexture(data["SheenTexturePath"]);
        mSheenRoughnessTexture  = tryLoadAsTexture(data["SheenRoughnessTexturePath"]);

        tryLoadAsVec3(data["DiffuseColour"], mDiffuseColour);
        tryLoadAsVec3(data["SpecularColour"], mSpecularColour);
        tryLoadAsFloat(data["Roughness"], mRoughness);
        tryLoadAsVec3(data["SheenColour"], mSheenColour);
        tryLoadAsFloat(data["SheenRoughness"], mSheenRoughness);
    }
} // engine
