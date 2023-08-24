/**
 * @file Viewport.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "ImGuizmo.h"

namespace engine
{

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
        
    protected:
        void onDrawUi() override;
        void toggleMouseState(bool newState);
        
        glm::vec2 mSize;
        bool mHideMouse { false };
        bool mIsHovered { false };
        bool mIsMouseDown { false };
        glm::dvec2 mLastMousePosition { 0.0, 0.0 };
        ImGuizmo::OPERATION mOperation { ImGuizmo::OPERATION::TRANSLATE };
        uint32_t mKeyboardEventToken { 0 };
        uint32_t mRightMouseEventToken { 0 };
        uint32_t mFirstPersonToken { 0 };
        uint32_t mThirdPersonToken { 0 };
        
        uint32_t mTranslateGizmoToken   { 0 };
        uint32_t mRotateGizmoToken      { 0 };
        uint32_t mScaleGizmoToken       { 0 };
    };
    
} // engine
