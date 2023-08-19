/**
 * @file Editor.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "Viewport.h"
#include "Actor.h"
#include "Callback.h"

namespace engine
{
/**
 * @author Ryan Purse
 * @date 08/08/2023
 */
    class Editor
        : public ui::Drawable
    {
    public:
        Callback<ImGuiKey, bool> onKeyPressed;
        Callback<ImGuiMouseButton, bool> onMouseClicked;  // true, if pressed. false, if released.
        
        void init();
        void update();
        [[nodiscard]] Actor *getSelectedActor();
        bool isViewportFocused();
        [[nodiscard]] float getMouseWheel();
    protected:
        void onDrawUi() override;
        void drawSceneHierarchyPanel();
        void drawActorDetails();
    
    protected:
        Viewport mViewport;
        Actor *mSelectedActor { nullptr };
        float mMouseWheel { 0.f };
    };
    
} // engine
