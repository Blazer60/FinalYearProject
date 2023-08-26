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
#include "LogWindow.h"

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
        void init();
        void update();
        [[nodiscard]] Actor *getSelectedActor();
        bool isViewportHovered();
        GLFWwindow *getViewportContext();
    
    protected:
        void onDrawUi() override;
        void drawSceneHierarchyPanel();
        void drawActorDetails();
        void createDefaultShape(const std::string& name, std::string_view path);
    
    protected:
        Viewport mViewport;
        LogWindow mLogWindow;
        Actor *mSelectedActor { nullptr };
    };
    
} // engine
