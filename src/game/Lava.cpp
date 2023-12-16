/**
 * @file Lava.cpp
 * @author Ryan Purse
 * @date 16/12/2023
 */


#include "Lava.h"


LavaMaterial::LavaMaterial()
    :
    mDiffuseMap(load::texture(file::texturePath() / "lava/TCom_Rock_Lava_1K_albedo.png")),
    mNormalMap(load::texture(file::texturePath() / "lava/TCom_Rock_Lava_1K_normal.png")),
    mHeightMap(load::texture(file::texturePath() / "lava/TCom_Rock_Lava_1K_height.png")),
    mRoughnessMap(load::texture(file::texturePath() / "lava/TCom_Rock_Lava_1K_roughness.png")),
    mEmissiveMap(load::texture(file::texturePath() / "lava/TCom_Rock_Lava_1K_emissive.png"))
{
    mShader = load::shader(
        file::shaderPath() / "geometry/standard/Standard.vert",
        file::shaderPath() / "geometry/lava/Lava.frag");
}

void LavaMaterial::onDraw()
{
    mShader->set("u_diffuse_texture", mDiffuseMap->id(), 0);
    mShader->set("u_normal_texture", mNormalMap->id(), 1);
    mShader->set("u_height_texture", mHeightMap->id(), 2);
    mShader->set("u_roughness_texture", mRoughnessMap->id(), 3);
    mShader->set("u_emissive_texture", mEmissiveMap->id(), 4);
    mShader->set("u_texture_offset", mOffset);
    mOffset += 0.01f * timers::deltaTime<float>();
}

void Lava::onDrawUi()
{

}

Material& Lava::getMaterial()
{
    return mLavaMaterial;
}
