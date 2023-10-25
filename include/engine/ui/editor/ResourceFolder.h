/**
 * @file ResourceFolder.h
 * @author Ryan Purse
 * @date 25/10/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"

namespace engine
{
    /**
     * @brief The name for places that accept images from imgui in the resource folder.
     */
    extern const char *resourceImagePayload;
    
    /**
     * @brief The name for places accept models from imgui payloads. See MeshRenderer for an example.
     */
    extern const char *resourceModelPayload;
    
/**
 * @author Ryan Purse
 * @date 25/10/2023
 */
    class ResourceFolder
        : public ui::Drawable
    {
    protected:
        void onDrawUi() override;
        void drawDirectory(const std::filesystem::path &path);
        
        std::filesystem::path mDragDropPath;
    };
} // engine
