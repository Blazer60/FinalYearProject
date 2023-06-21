/**
 * @file Materials.cpp
 * @author Ryan Purse
 * @date 15/06/2023
 */


#include "Materials.h"

#include <utility>

void Material::attachShader(std::shared_ptr<Shader> shader)
{
    mShader = std::move(shader);
}

void SimpleMaterial::onLoadMtlFile(const MtlMaterialInformation &materialInformation)
{
    mColour = materialInformation.kD;
}

void SimpleMaterial::onDraw()
{
    mShader->set("u_colour", mColour);
}
