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
        const std::string tempFilePath = "temp.pcy";
    public:
        Core(const glm::ivec2 &resolution, bool enableDebugging);
        ~Core();
        
        void setScene(std::unique_ptr<Scene> scene, const std::filesystem::path &path);
        void run();
        void beginPlay();
        void endPlay();

        [[nodiscard]] Scene *getScene() const;
        [[nodiscard]] EditorCamera *getCamera() const;
        [[nodiscard]] std::string getSceneName();
        [[nodiscard]] std::filesystem::path getScenePath() const;
        [[nodiscard]] btDiscreteDynamicsWorld *getPhysicsWorld() const;
        bool isInPlayMode() const;
        void setScenePath(std::filesystem::path path);

    protected:
        bool initGlfw(int openGlMajorVersion, int openGlMinorVersion);
        bool initImGui();
        void initOpenAL();
        void updateImgui();
        static void configureUiThemeColours(ImGuiStyle &style) ;
    
    protected:
        const glm::ivec2 mResolution { 1920, 1080 };
        const std::string_view mWindowTitle { "Game Engine" };
        
        GLFWimage mWindowIcon{};
        GLFWwindow *mWindow { nullptr };
        ALCdevice *mAudioDevice { nullptr };
        ALCcontext *mAudioContext { nullptr };

        std::unique_ptr<Scene> mScene;
        Scene *mScenePointer { nullptr };
        std::filesystem::path mScenePath;
        std::string mSceneName;
        
        std::unique_ptr<ResourcePool> mResourcePool;
        std::unique_ptr<Renderer> mRenderer;
        std::unique_ptr<debug::Logger> mLogger;
        std::unique_ptr<Profiler> mProfiler;
        std::unique_ptr<Serializer> mSerializer;
        
        ImGuiIO *mGuiIo { nullptr };
        bool mIsRunning { true };
        bool mIsInPlayMode { false };
        const unsigned int mMaxLoopCount { 10 };
        const bool mEnableDebugging { false };
        
        std::unique_ptr<EditorCamera> mEditorCamera;
        Ref<Camera> mPlayModeCamera;
        
        std::unique_ptr<Editor> mEditor;
        RootEventHandler mEventHandler;

        std::unique_ptr<PhysicsCore> mPhysics;
    };
    
} // engine
