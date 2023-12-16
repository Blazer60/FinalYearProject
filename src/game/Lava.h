/**
 * @file Lava.h
 * @author Ryan Purse
 * @date 16/12/2023
 */


#pragma once

#include "Pch.h"
#include <Engine.h>

class LavaMaterial
    : public Material
{
public:
    LavaMaterial();
    void onDraw() override;

protected:
    std::shared_ptr<Texture> mDiffuseMap;
    std::shared_ptr<Texture> mNormalMap;
    std::shared_ptr<Texture> mHeightMap;
    std::shared_ptr<Texture> mRoughnessMap;
    std::shared_ptr<Texture> mEmissiveMap;
    engine::vec2 mOffset { 0.f };
};

/**
 * @author Ryan Purse
 * @date 16/12/2023
 */
class Lava
    : public engine::MaterialSubComponent
{
protected:
    void onDrawUi() override;

public:
    Material& getMaterial() override;
protected:
    LavaMaterial mLavaMaterial;
};
