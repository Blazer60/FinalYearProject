/**
 * @file Materials.cpp
 * @author Ryan Purse
 * @date 15/06/2023
 */


#include "Materials.h"
#include "TextureLoader.h"
#include "gtc/type_ptr.hpp"
#include "Ui.h"

#include <utility>

void Material::attachShader(std::shared_ptr<Shader> shader)
{
    mShader = std::move(shader);
}

void Material::onDrawUi()
{

}

StandardMaterial::StandardMaterial() :
    mDiffuse(std::make_shared<Texture>("")),
    mNormal(std::make_shared<Texture>("")),
    mHeight(std::make_shared<Texture>("")),
    mRoughnessMap(std::make_shared<Texture>("")),
    mMetallicMap(std::make_shared<Texture>(""))
{
    // We create empty textures so that the shader can still bind to slot zero.
}

void StandardMaterial::onDraw()
{
    mShader->set("u_ambient_colour", ambientColour);
    mShader->set("u_diffuse_texture", mDiffuse->id(), 1);
    mShader->set("u_normal_texture", mNormal->id(), 2);
    mShader->set("u_height_texture", mHeight->id(), 3);
    mShader->set("u_roughness_texture", mRoughnessMap->id(), 4);
    mShader->set("u_metallic_texture", mMetallicMap->id(), 5);
    mShader->set("u_height_scale", heightScale);
    mShader->set("u_min_height_samples", minHeightSamples);
    mShader->set("u_max_height_samples", maxHeightSamples);
    mShader->set("u_roughness", roughness);
    mShader->set("u_metallic", metallic);
    mShader->set("u_emissive_colour", emissive);
}

void StandardMaterial::onLoadMtlFile(const MtlMaterialInformation &materialInformation)
{
    ambientColour = materialInformation.kD;
    mDiffuse = load::texture(materialInformation.mapKd);
    mNormal = load::texture(materialInformation.mapBump);
}

void StandardMaterial::setHeightMap(std::shared_ptr<Texture> heightMap)
{
    mHeight = std::move(heightMap);
}

void StandardMaterial::setRoughnessMap(std::shared_ptr<Texture> roughnessMap)
{
    mRoughnessMap = std::move(roughnessMap);
}

void StandardMaterial::setDiffuseMap(std::shared_ptr<Texture> diffuseMap)
{
    mDiffuse = std::move(diffuseMap);
}

void StandardMaterial::setNormalMap(std::shared_ptr<Texture> normalMap)
{
    mNormal = std::move(normalMap);
}

void StandardMaterial::setMetallicMap(std::shared_ptr<Texture> metallicMap)
{
    mMetallicMap = std::move(metallicMap);
}

void StandardMaterial::onDrawUi()
{
    std::string id = "StandardMaterialSettings" + std::to_string((uint64_t)this);
    ImGui::PushID(id.c_str());
    if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        
        const ImVec2 imageSize = ImVec2(15.f, 15.f);
        
        ImGui::ImageButton("Diffuse Texture", reinterpret_cast<void *>(mDiffuse->id()), imageSize, ImVec2(0, 1), ImVec2(1, 0));
        ui::drawToolTip("Albedo Texture");
        ImGui::SameLine();
        ImGui::ColorEdit3("Albedo", glm::value_ptr(ambientColour), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
        
        ImGui::ImageButton("Roughness Texture", reinterpret_cast<void *>(mRoughnessMap->id()), imageSize, ImVec2(0, 1), ImVec2(1, 0));
        ui::drawToolTip("Roughness Texture");
        ImGui::SameLine();
        ImGui::SliderFloat("Roughness", &roughness, 0.f, 1.f);
        
        ImGui::ImageButton("Metallic Texture", reinterpret_cast<void *>(mMetallicMap->id()), imageSize, ImVec2(0, 1), ImVec2(1, 0));
        ui::drawToolTip("Metallic Texture");
        ImGui::SameLine();
        ImGui::SliderFloat("Metallic", &metallic, 0.f, 1.f);
        
        ImGui::ColorEdit3("Emissive Colour", glm::value_ptr(emissive), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
        ImGui::SliderFloat("Height", &heightScale, 0.f, 1.f);
        
        ImGui::TreePop();
    }
    ImGui::PopID();
}




