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

#include "Camera.h"
#include "EditorCamera.h"
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
        Viewport();
        ~Viewport() override;
        
        void init();
        [[nodiscard]] glm::vec2 getSize() const;

        void drawTopBar();

        void drawEditorView();

        [[nodiscard]] bool isHovered() const;
        [[nodiscard]] bool isUsingPlayModeCamera() const;
        GLFWwindow *getViewportContext() const;
        bool isDebugViewOn() const;
        void preRender();
        void update();
        void beginPlay();
        EditorCamera *getCamera();

        bool isShowing { true };
    protected:
        void drawPlayModeView() const;

        void onDrawUi() override;
        void toggleMouseState(bool newState) const;
        
        glm::vec2 mSize { 0.f };
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
        bool mShowTileClassification { false };
        bool mIsSimulating { false };

        EditorCamera mEditorCamera;
        Ref<Camera> mPlayModeCamera;

        Texture mPlayButton { file::texturePath() / "PlayButton.png" };
        Texture mStopButton { file::texturePath() / "PauseButton.png" };
    };
    
} // engine
