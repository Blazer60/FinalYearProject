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
    extern const char *resourceImagePayload;
    
    /**
     * @brief The name for places that accept models from imgui payloads. See MeshRenderer for an example.
     */
    extern const char *resourceModelPayload;
    
    /**
     * @brief The name for places that accept scenes from imgui payloads.
     */
    extern const char *resourceScenePayload;

    /**
     * @brief The name for places that accept sounds from imgui payloads.
     */
    extern const char *resourceSoundPayload;
    
/**
 * @author Ryan Purse
 * @date 25/10/2023
 */
    class ResourceFolder
        : public ui::Drawable
    {
    public:
        bool isShowing { true };
    protected:
        void onDrawUi() override;

        void drawDragDropSource(const std::filesystem::path&path, const std::string&name);

        void drawDirectory(const std::filesystem::path &path);
        void drawContents();
        void drawContentItem(const std::filesystem::directory_entry& item);

        std::filesystem::path mSelectedFolder = file::resourcePath();
        std::filesystem::path mDragDropPath;

        Texture mFolderIconTexture = Texture(file::texturePath() / "FolderIcon.png");
        Texture mUnknownIconTexture = Texture(file::texturePath() / "ObjectIcon.png");
        float mItemSize = 100.f;
    };
} // engine
