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
#include "EngineMemory.h"

namespace engine
{
    struct ComponentDetails
    {
        typedef std::function<void(Ref<Actor>)> CreateFunc;
        
        ComponentDetails(std::string previewName, CreateFunc onCreate)
            : previewName(std::move(previewName)), onCreate(std::move(onCreate))
        {
        }
        virtual ~ComponentDetails() = default;
        
        std::string previewName;
        std::function<void(Ref<Actor>)> onCreate;
        
        virtual bool hasThisComponent(const Ref<Actor>& actor) = 0;
    };
    
    template<typename T>
    struct CreateComponentDetails : public ComponentDetails
    {
        CreateComponentDetails(std::string previewName, CreateFunc onCreate)
            : ComponentDetails(previewName, onCreate)
        {
        
        }
        
        bool hasThisComponent(const Ref<Actor>& actor) override
        {
            return actor->hasComponent<T>();
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
        [[nodiscard]] Ref<Actor> getSelectedActor();
        bool isViewportHovered();
        GLFWwindow *getViewportContext();
        
        template<typename T>
        void addComponentOption(const std::string &name, const ComponentDetails::CreateFunc &onCreate);
    protected:
        void onDrawUi() override;
        void drawSceneHierarchyPanel();
        void drawSceneHierarchyForActor(Ref<Actor> &actor);
        void drawActorDetails();
        void drawAddComponentCombo();
        void createDefaultShape(const std::string& name, std::string_view path);
    
    protected:
        Viewport mViewport;
        LogWindow mLogWindow;
        Ref<Actor> mSelectedActor;
        std::vector<std::unique_ptr<ComponentDetails>> mComponentList;
        
        uint64_t mDeletionToken { 0 };
    };
    
    
    template<typename T>
    void Editor::addComponentOption(const std::string &name, const ComponentDetails::CreateFunc &onCreate)
    {
        mComponentList.push_back(std::make_unique<CreateComponentDetails<T>>(name, onCreate));
    }
} // engine
