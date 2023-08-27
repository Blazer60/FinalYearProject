/**
 * @file Editor.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include <utility>

#include "Pch.h"
#include "Drawable.h"
#include "Viewport.h"
#include "Actor.h"
#include "Callback.h"
#include "LogWindow.h"

namespace engine
{
    struct ComponentDetails
    {
        ComponentDetails(std::string previewName, std::function<void(Actor&)> onCreate)
            : previewName(std::move(previewName)), onCreate(std::move(onCreate))
        {
        }
        virtual ~ComponentDetails() = default;
        
        std::string previewName;
        std::function<void(Actor&)> onCreate;
        
        virtual bool hasThisComponent(Actor& actor) = 0;
    };
    
    template<typename T>
    struct CreateComponentDetails : public ComponentDetails
    {
        CreateComponentDetails(std::string previewName, std::function<void(Actor&)> onCreate)
            : ComponentDetails(previewName, onCreate)
        {
        
        }
        
        bool hasThisComponent(Actor &actor) override
        {
            return actor.hasComponent<T>();
        }
    };
    
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
        
        template<typename T>
        void addComponentOption(const std::string &name, const std::function<void(Actor&)> &onCreate);
        
        void attachSceneCallbacks(Scene *scene);
        void detachSceneCallbacks() const;
    
    protected:
        void onDrawUi() override;
        void drawSceneHierarchyPanel();
        void drawActorDetails();
        void drawAddComponentCombo();
        void createDefaultShape(const std::string& name, std::string_view path);
    
    protected:
        Viewport mViewport;
        LogWindow mLogWindow;
        Actor *mSelectedActor { nullptr };
        std::vector<std::unique_ptr<ComponentDetails>> mComponentList;
        
        uint64_t mDeletionToken { 0 };
    };
    
    
    template<typename T>
    void Editor::addComponentOption(const std::string &name, const std::function<void(Actor &)> &onCreate)
    {
        mComponentList.push_back(std::make_unique<CreateComponentDetails<T>>(name, onCreate));
    }
} // engine
