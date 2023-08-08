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
        [[nodiscard]] Actor *getSelectedActor();
    protected:
        void onDrawUi() override;
        void drawSceneHierarchyPanel();
        void drawActorDetails();
    
    protected:
        Viewport mViewport;
        Actor *mSelectedActor { nullptr };
    };
    
} // engine
