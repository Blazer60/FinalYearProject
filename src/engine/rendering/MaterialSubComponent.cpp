/**
 * @file MaterialSubComponent.cpp
 * @author Ryan Purse
 * @date 17/10/2023
 */


#include "MaterialSubComponent.h"
#include "TextureLoader.h"
#include "Ui.h"
#include "FileExplorer.h"
#include "EngineState.h"
#include "Editor.h"

namespace engine
{
    void StandardMaterialSubComponent::setDiffuseMap(const std::filesystem::path &diffuseMapPath)
    {
        auto texture = load::texture(diffuseMapPath);
        mDiffuseMapPath = diffuseMapPath.string();
        mMaterial.setDiffuseMap(std::move(texture));
    }
    
    void StandardMaterialSubComponent::setNormalMap(const std::filesystem::path &normalMapPath)
    {
        auto texture = load::texture(normalMapPath);
        mNormalMapPath = normalMapPath.string();
        mMaterial.setNormalMap(std::move(texture));
    }
    
    void StandardMaterialSubComponent::setHeightMap(const std::filesystem::path &heightMapPath)
    {
        auto texture = load::texture(heightMapPath);
        mHeightMapPath = heightMapPath.string();
        mMaterial.setHeightMap(std::move(texture));
    }
    
    void StandardMaterialSubComponent::setRoughnessMap(const std::filesystem::path &roughnessMapPath)
    {
        auto texture = load::texture(roughnessMapPath);
        mRoughnessMapPath = roughnessMapPath.string();
        mMaterial.setRoughnessMap(std::move(texture));
    }
    
    void StandardMaterialSubComponent::setMetallicMap(const std::filesystem::path &metallicMapPath)
    {
        auto texture = load::texture(metallicMapPath);
        mMetallicMapPath = metallicMapPath.string();
        mMaterial.setMetallicMap(std::move(texture));
    }
    
    void StandardMaterialSubComponent::onDrawUi()
    {
        std::string id = "StandardMaterialSettings" + std::to_string((uint64_t)this);
        ImGui::PushID(id.c_str());
        if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            const glm::vec2 imageSize = glm::vec2(15.f, 15.f);
            
            if (ImGui::Button("X##Diffuse"))
            {
                engine::editor->addUpdateAction([this]() { setDiffuseMap(""); });
            }
            ImGui::SameLine();
            if (ui::imageButton("Diffuse texture", mMaterial.diffuseMapId(), imageSize))
            {
                engine::editor->addUpdateAction([this]() {
                    const std::string result = openFileExplorer();
                    if (result.empty())
                        return;
                    
                    setDiffuseMap(result);
                });
            }
            ui::drawToolTip("Albedo Texture");
            ImGui::SameLine();
            ImGui::ColorEdit3("Albedo", glm::value_ptr(mMaterial.ambientColour), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
            
            if (ImGui::Button("X##Normal"))
            {
                engine::editor->addUpdateAction([this]() { setNormalMap(""); });
            }
            ImGui::SameLine();
            if (ui::imageButton("Normal Texture", mMaterial.normalMapId(), imageSize))
            {
                engine::editor->addUpdateAction([this]() {
                    const std::string result = openFileExplorer();
                    if (result.empty())
                        return;
                    
                    setNormalMap(result);
                });
            }
            ui::drawToolTip("Normal Map");
            
            if (ImGui::Button("X##Roughness"))
            {
                engine::editor->addUpdateAction([this]() { setRoughnessMap(""); });
            }
            ImGui::SameLine();
            if (ui::imageButton("Roughness Texture", mMaterial.roughnessMapId(), imageSize))
            {
                engine::editor->addUpdateAction([this]() {
                    const std::string result = openFileExplorer();
                    if (result.empty())
                        return;
                    
                    setRoughnessMap(result);
                });
            }
            ui::drawToolTip("Roughness Texture");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Roughness").x);
            ImGui::SliderFloat("Roughness", &mMaterial.roughness, 0.f, 1.f);
            
            if (ImGui::Button("X##Metallic"))
            {
                engine::editor->addUpdateAction([this]() { setMetallicMap(""); });
            }
            ImGui::SameLine();
            if (ui::imageButton("Metallic Texture", mMaterial.metallicMapId(), imageSize))
            {
                engine::editor->addUpdateAction([this]() {
                    const std::string result = openFileExplorer();
                    if (result.empty())
                        return;
                    
                    setMetallicMap(result);
                });
            }
            ui::drawToolTip("Metallic Texture");
            ImGui::SameLine();
            ImGui::SliderFloat("Metallic", &mMaterial.metallic, 0.f, 1.f);
            
            ImGui::ColorEdit3("Emissive Colour", glm::value_ptr(mMaterial.emissive), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
            
            if (ImGui::Button("X##Height"))
            {
                engine::editor->addUpdateAction([this]() { setHeightMap(""); });
            }
            ImGui::SameLine();
            if (ui::imageButton("Height Texture", mMaterial.heightMapId(), imageSize))
            {
                engine::editor->addUpdateAction([this]() {
                    const std::string result = openFileExplorer();
                    if (result.empty())
                        return;
                    
                    setHeightMap(result);
                });
            }
            ui::drawToolTip("Height Map Texture");
            ImGui::SameLine();
            ImGui::SliderFloat("Height", &mMaterial.heightScale, 0.f, 1.f);
            
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    
    void StandardMaterialSubComponent::setMetallic(float value)
    {
        mMaterial.metallic = value;
    }
    
    void StandardMaterialSubComponent::setRoughness(float value)
    {
        mMaterial.roughness = value;
    }
    
    void StandardMaterialSubComponent::setAmbientColour(const glm::vec3 &colour)
    {
        mMaterial.ambientColour = colour;
    }
    
    void MaterialSubComponent::attachShader(const std::shared_ptr<Shader> &shader)
    {
        getMaterial().attachShader(shader);
    }
}