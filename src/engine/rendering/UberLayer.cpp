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

    }

    void UberLayer::onDrawUi()
    {
        ui::inputText("##name", &mName);
        const auto name = format::string("%", mPath);
        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), name.c_str());

        ImGui::SeparatorText("Default Settings");
        ui::textureColourEdit("Diffuse", mDiffuseTexture, mDiffuseColour);
        ui::textureColourEdit("Specular", mSpecularTexture, mSpecularColour);
        ui::texture("Normal Map", mNormalTexture);
        ui::textureSliderFloat("Roughness", mRoughnessTexture, mRoughness);

        ImGui::SeparatorText("Sheen Settings");
        ui::textureColourEdit("Sheen Colour", mSheenTexture, mSheenColour);
        ui::textureSliderFloat("Sheen Roughness", mSheenRoughnessTexture, mSheenRoughness);

        if (ImGui::Button("Save"))
        {
            MESSAGE("Todo: Save to disk");
        }
    }
} // engine
