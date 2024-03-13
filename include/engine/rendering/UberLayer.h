/**
 * @file UberLayer.h
 * @author Ryan Purse
 * @date 11/03/2024
 */


#pragma once

#include "Callback.h"
#include "Drawable.h"
#include "FileLoader.h"
#include "Pch.h"
#include "Texture.h"

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 11/03/2024
     */
    class UberLayer
        : public ui::Drawable
    {
        friend class UberMaterial;
    public:
        explicit UberLayer(const std::filesystem::path &path);
        std::string name() const { return mName; }
        std::filesystem::path path() const { return mPath; }
        void onDrawUi() override;
        // Saving is handled by the resource pool once the last instance has gone out of scope.
        void saveToDisk() const;


    protected:
        void loadFromDisk();

        std::string mName;
        std::filesystem::path mPath;

        std::shared_ptr<Texture> mDiffuseTexture = std::make_shared<Texture>("");
        std::shared_ptr<Texture> mSpecularTexture = std::make_shared<Texture>("");
        std::shared_ptr<Texture> mNormalTexture = std::make_shared<Texture>("");
        std::shared_ptr<Texture> mRoughnessTexture = std::make_shared<Texture>("");
        std::shared_ptr<Texture> mSheenTexture = std::make_shared<Texture>("");
        std::shared_ptr<Texture> mSheenRoughnessTexture = std::make_shared<Texture>("");

        glm::vec3 mDiffuseColour = glm::vec3(0.8f, 0.8f, 0.8f);
        glm::vec3 mSpecularColour = glm::vec3(0.2f, 0.2f, 0.2f);
        float     mRoughness = 0.9f;
        glm::vec3 mSheenColour = glm::vec3(0.f);
        float     mSheenRoughness = 0.5f;
    };

} // engine
