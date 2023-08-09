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
        Callback<std::vector<ImGuiKey>> onIoKeyboardEvent;
        Callback<bool> onRightMouseClicked;  // true, if pressed. false, if released.
        
        void init();
        void update();
        [[nodiscard]] Actor *getSelectedActor();
        bool isViewportFocused();
    protected:
        void onDrawUi() override;
        void drawSceneHierarchyPanel();
        void drawActorDetails();
    
    protected:
        Viewport mViewport;
        Actor *mSelectedActor { nullptr };
    };
    
} // engine
