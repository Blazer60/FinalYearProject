/**
 * @file UberMaterial.h
 * @author Ryan Purse
 * @date 11/03/2024
 */


#pragma once

#include "Drawable.h"
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
        void onDrawUi() override;
        void saveToDisk() const;

    protected:
        void loadFromDisk();
        void drawMaterialLayerArray();

        void moveElementInPlace(int srcIndex, int dstIndex);

        bool drawMaterialLayerElement(int index);
        std::string mName;
        std::filesystem::path mPath;

        std::vector<std::shared_ptr<UberLayer>> mLayers;
    };
} // engine
