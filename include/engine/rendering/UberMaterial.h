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
#include "TexturePool.h"
#include "UberLayer.h"
#include "UberMask.h"

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
        bool empty() const { return mLayers.empty(); }

        void drawMaskArray();

        void onDrawUi() override;
        void saveToDisk() const;

        void onPreRender();
        graphics::MaterialData getData() { return mData; }

    protected:
        void loadFromDisk();
        void drawMaterialLayerArray();
        void drawMaterialLayerElementColumn(const std::string&name, int index);
        bool drawMaterialLayerElement(int index);
        bool drawMaskLayerElement(int index);
        void updateGraphicsData();
        void addMask(std::unique_ptr<UberMask> mask);
        void destroyMask(int index);
        void addNewMaterialLayer(std::shared_ptr<UberLayer> layer);
        void addNewMaterialLayer(std::shared_ptr<UberLayer> layer, int index);
        void destroyLayer(int index);


        std::string mName;
        std::filesystem::path mPath;

        std::vector<std::shared_ptr<UberLayer>> mLayers;
        std::vector<std::unique_ptr<UberMask>> mMasks;
        graphics::TexturePool mTexturePool = graphics::TexturePool(mPath.string(), graphics::textureFormat::Rgba8, 8);
        graphics::MaterialData mData;
    };
} // engine
