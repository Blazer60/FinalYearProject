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
