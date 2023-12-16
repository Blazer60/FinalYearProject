/**
 * @file Core.h
 * @author Ryan Purse
 * @date 12/06/2023
 */


#pragma once

#include "Pch.h"
#include "glfw3.h"
#include "imgui.h"
#include "EventHandler.h"
#include "TextureBufferObject.h"
#include "Renderer.h"
#include "Logger.h"
#include "EditorCamera.h"
#include "Viewport.h"
#include "Editor.h"
#include "ProfilerViewer.h"
#include "ResourcePool.h"

#include <al.h>
#include <alc.h>

#include "PhysicsCore.h"

namespace engine
{
    class Scene;
    
    /**
     * @author Ryan Purse
     * @date 12/06/2023
     */
    class Core
    {
        inline static std::string tempFilePath = "temp.pcy";
    public:
        /**
         * \param resolution The resolution of the window to begin with (The window is resizable).
         * \param enableDebugging Should debugging for graphics and audio be enabled?
         */
        Core(const glm::ivec2 &resolution, bool enableDebugging);
        ~Core();

        /**
         * \brief Sets the scene that you want to use. Optional setup step.
         * \param scene The new scene that you want to set.
         * \param path The path to the new scene. Can be left blank to force no current save path.
         */
        void setScene(std::unique_ptr<Scene> scene, const std::filesystem::path &path="");

        /**
         * \brief Run the application until the user closes the window.
         */
        void run();

        /**
         * \brief Runs the scene in play mode which allows many actors to update.
         */
        void beginPlay();

        /**
         * \brief Returns the scene back to edit mode.
         */
        void endPlay();

        [[nodiscard]] Scene *getScene() const;
        [[nodiscard]] std::string getSceneName();
        [[nodiscard]] std::filesystem::path getScenePath() const;
        [[nodiscard]] btDiscreteDynamicsWorld *getPhysicsWorld() const;
        [[nodiscard]] bool isInPlayMode() const;
        void setScenePath(std::filesystem::path path);

    protected:
        bool initGlfw(int openGlMajorVersion, int openGlMinorVersion);
        bool initImGui();
        void initOpenAL();
        void updateImgui();
        static void configureUiThemeColours(ImGuiStyle &style) ;
    
        const glm::ivec2 mResolution        { 1920, 1080 };
        const std::string_view mWindowTitle { "Game Engine" };
        
        GLFWimage    mWindowIcon     { };
        GLFWwindow  *mWindow         { nullptr };
        ALCdevice   *mAudioDevice    { nullptr };
        ALCcontext  *mAudioContext   { nullptr };

        std::unique_ptr<Scene> mScene;
        Scene *mScenePointer { nullptr };
        std::filesystem::path mScenePath;
        std::string mSceneName;
        
        std::unique_ptr<ResourcePool>   mResourcePool;
        std::unique_ptr<Renderer>       mRenderer;
        std::unique_ptr<debug::Logger>  mLogger;
        std::unique_ptr<Profiler>       mProfiler;
        std::unique_ptr<Serializer>     mSerializer;
        std::unique_ptr<Editor>         mEditor;
        std::unique_ptr<PhysicsCore>    mPhysics;
        RootEventHandler                mEventHandler;

        ImGuiIO *mGuiIo         { nullptr };
        bool     mIsRunning     { true };
        bool     mIsInPlayMode  { false };  // todo: Is this more of an editor thing?

        const unsigned int mMaxLoopCount { 10 };
        const bool mEnableDebugging { false };
    };
    
} // engine
