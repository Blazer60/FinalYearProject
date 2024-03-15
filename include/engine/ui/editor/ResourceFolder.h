/**
 * @file ResourceFolder.h
 * @author Ryan Purse
 * @date 25/10/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "FileLoader.h"
#include "Texture.h"

namespace engine
{
    /**
     * @brief The name for places that accept images from imgui in the resource folder.
     */
    extern const char *const resourceImagePayload;
    
    /**
     * @brief The name for places that accept models from imgui payloads. See MeshRenderer for an example.
     */
    extern const char *const resourceModelPayload;
    
    /**
     * @brief The name for places that accept scenes from imgui payloads.
     */
    extern const char *const resourceScenePayload;

    /**
     * @brief The name for places that accept sounds from imgui payloads.
     */
    extern const char *const resourceSoundPayload;

    extern const char *const resourceMaterialLayerPayload;

    extern const char *const resourceMaterialPayload;

/**
 * @author Ryan Purse
 * @date 25/10/2023
 */
    class ResourceFolder
        : public ui::Drawable
    {
    public:
        ResourceFolder();
        bool isShowing { true };
    protected:
        void onDrawUi() override;

        void drawDragDropSource(const std::filesystem::path&path, const std::string&name);
        void drawDirectory(const std::filesystem::path &path);
        void drawMaterialLayerModal(bool toggleMaterialLayerPopup);
        void drawMaterialModal(bool toggleMaterialPopup);
        void drawContents();
        void drawContentItem(const std::filesystem::directory_entry& item);
        void userSelectAction(const std::filesystem::path &path);

        void changeContentsFolder(const std::filesystem::path &path);

        std::filesystem::path mSelectedFolder = file::resourcePath();
        std::filesystem::path mDragDropPath;

        Texture mFolderIconTexture = Texture(file::texturePath() / "FolderIcon.png");
        Texture mUnknownIconTexture = Texture(file::texturePath() / "ObjectIcon.png");
        float mItemSize = 100.f;
        std::string mNewFileName;

        std::unordered_map<std::string, std::shared_ptr<Texture>> mTextureIcons;
    };
} // engine
