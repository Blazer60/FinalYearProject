/**
 * @file Viewport.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "ImGuizmo.h"
#include "TextureBufferObject.h"
#include <glfw3.h>

#include "FileLoader.h"
#include "Texture.h"

namespace engine
{
    struct ViewportImage
    {
        std::string name;
        std::function<TextureBufferObject const&()> requestTexture;
    };

/**
 * @author Ryan Purse
 * @date 08/08/2023
 */
    class Viewport
        : public ui::Drawable
    {
    public:
        void init();
        ~Viewport() override;
        
        [[nodiscard]] glm::vec2 getSize() const;
        [[nodiscard]] bool isHovered() const;
        GLFWwindow *getViewportContext();
        bool isDebugViewOn() const;

        bool isShowing { true };
    protected:
        void onDrawUi() override;
        void toggleMouseState(bool newState);
        
        glm::vec2 mSize;
        bool mIsHovered { false };
        glm::dvec2 mLastMousePosition { 0.0, 0.0 };
        ImGuizmo::OPERATION mOperation { ImGuizmo::OPERATION::TRANSLATE };
        ImGuizmo::MODE mMode { ImGuizmo::MODE::LOCAL };
        uint32_t mFirstPersonToken { 0 };
        uint32_t mThirdPersonToken { 0 };
        
        uint32_t mTranslateGizmoToken   { 0 };
        uint32_t mRotateGizmoToken      { 0 };
        uint32_t mScaleGizmoToken       { 0 };
        
        GLFWwindow *mViewportWindow { nullptr };
        
        std::vector<ViewportImage> mViewportImages;
        int32_t mCurrentSelectedImage = 0;
        bool mForce1080p { false };
        bool mShowDebugOverlay { false };

        Texture mPlayButton { file::texturePath() / "PlayButton.png" };
        Texture mStopButton { file::texturePath() / "PauseButton.png" };
    };
    
} // engine
