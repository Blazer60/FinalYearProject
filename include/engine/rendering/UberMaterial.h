/**
 * @file UberMaterial.h
 * @author Ryan Purse
 * @date 11/03/2024
 */


#pragma once

#include "Drawable.h"
#include "GraphicsFunctions.h"
#include "MaterialData.h"
#include "Pch.h"
#include "UberLayer.h"

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 11/03/2024
     */
    class UberMaterial
        : public ui::Drawable
    {
    public:
        explicit UberMaterial(const std::filesystem::path &path);
        std::string name() const { return mName; }
        std::filesystem::path path() const { return mPath; };
        void onDrawUi() override;
        void saveToDisk() const;

        void onPreRender();
        graphics::MaterialData getData() { return mData; }

    protected:
        void loadFromDisk();
        void drawMaterialLayerArray();

        void drawMaterialLayerElementColumn(const std::string&name, int index);

        bool drawMaterialLayerElement(int index);
        std::string mName;
        std::filesystem::path mPath;

        std::vector<std::shared_ptr<UberLayer>> mLayers;
        TextureArrayObject mTextureArray = TextureArrayObject(graphics::textureFormat::Rgba8, graphics::filter::LinearMipmapLinear);
        graphics::MaterialData mData;
    };
} // engine
