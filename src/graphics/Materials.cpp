/**
 * @file Materials.cpp
 * @author Ryan Purse
 * @date 15/06/2023
 */


#include "Materials.h"
#include "TextureLoader.h"

#include <utility>

void Material::attachShader(std::shared_ptr<Shader> shader)
{
    mShader = std::move(shader);
}

void SimpleMaterial::onLoadMtlFile(const MtlMaterialInformation &materialInformation)
{
    mColour = materialInformation.kD;
    mTexture = load::texture(materialInformation.mapKd);
}

void SimpleMaterial::onDraw()
{
    mShader->set("u_colour", mColour);
    mShader->set("u_texture", mTexture->id(), 1);
}

void StandardMaterial::onDraw()
{
    mShader->set("u_ambient_colour", mAmbientColour);
    mShader->set("u_diffuse_texture", mDiffuse->id(), 1);
}

void StandardMaterial::onLoadMtlFile(const MtlMaterialInformation &materialInformation)
{
    mAmbientColour = materialInformation.kD;
    mDiffuse = load::texture(materialInformation.mapKd);
}
