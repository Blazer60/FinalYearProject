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
#include "ResourceFolder.h"
#include "ProfilerViewer.h"

namespace engine
{
    struct ActorDetails
    {
        typedef std::function<Ref<Actor>(void)> CreateFunc;
        
        std::string previewName;
        CreateFunc onCreate;
    };
    
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
        ~Editor() override;
        void init();
        void update();
        void preRender();
        
        [[nodiscard]] Ref<Actor> getSelectedActor();
        bool isViewportHovered();
        bool isDebugOverlayOn() const;
        bool isUsingPlayModeCamera() const;
        GLFWwindow *getViewportContext();
        
        template<typename T>
        void addComponentOption(const std::string &name, const ComponentDetails::CreateFunc &onCreate);
        void addMenuOption(const std::string &name, const ActorDetails::CreateFunc &onCreate);

        Ref<Actor> createDefaultShape(const std::string& name, std::string_view path);
        void createModel(const std::filesystem::path &path);
        
        /**
         * @brief Allows callbacks to processed in the update function. This is mainly to stop imgui
         * referencing deleted data. Only to be used as a last resort. If an object already has an
         * onUpdate function, use that instead.
         */
        void addUpdateAction(const std::function<void()> &callback);

        void relinkSelectedActor();
    protected:
        void onDrawUi() override;
        void drawSceneHierarchyPanel();
        void drawSceneHierarchyForActor(Ref<Actor> actor);
        void drawActorDetails();
        void drawAddComponentCombo();
        void drawSceneSettings();
        void drawFileMenuDropDown();
        void drawWindowDropDown();
        void drawMenuBar();
        void moveActors();

        void setSelectedActor(Ref<Actor> actor);

        Viewport mViewport;
        LogWindow mLogWindow;
        ResourceFolder mResourceFolder;
        ProfilerViewer mProfilerViewer;
        Ref<Actor> mSelectedActor;
        UUID mSelectedActorId { 0 };
        std::vector<ActorDetails> mMenuList;
        std::vector<std::unique_ptr<ComponentDetails>> mComponentList;

        uint32_t mDeleteActorToken { 0 };
        
        Actor* mMoveSourceActor { nullptr };
        Actor* mMoveDestinationActor { nullptr };
        
        std::vector<std::function<void()>> mOnUpdate;

        bool mShowSceneHierarchy { true };
        bool mShowDetailsPanel   { true };
        bool mShowSceneSettings  { true };
        bool mShowCameraSettings { true };
    };
    
    
    template<typename T>
    void Editor::addComponentOption(const std::string &name, const ComponentDetails::CreateFunc &onCreate)
    {
        mComponentList.push_back(std::make_unique<CreateComponentDetails<T>>(name, onCreate));
    }
} // engine
