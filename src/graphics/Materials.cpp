/**
 * @file Materials.cpp
 * @author Ryan Purse
 * @date 15/06/2023
 */


#include "Materials.h"
#include "TextureLoader.h"
#include "gtc/type_ptr.hpp"

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
    mRoughnessMap(std::make_shared<Texture>(""))
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

void StandardMaterial::onDrawUi()
{
    ImGui::PushID("StandardMaterialSettings");
    if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
    {
        ImGui::ColorEdit3("Ambient Colour", glm::value_ptr(ambientColour));
        ImGui::SliderFloat("Roughness", &roughness, 0.f, 1.f);
        ImGui::SliderFloat("Metallic", &metallic, 0.f, 1.f);
        ImGui::ColorEdit3("Emissive Colour", glm::value_ptr(emissive), ImGuiColorEditFlags_HDR);
        
        ImGui::TreePop();
    }
    ImGui::PopID();
}



