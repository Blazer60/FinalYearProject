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
#include "MainCamera.h"
#include "Viewport.h"
#include "Editor.h"
#include "ProfilerViewer.h"
#include "ResourcePool.h"

namespace engine
{
    class Scene;
    
/**
 * @author Ryan Purse
 * @date 12/06/2023
 */
    class Core
    {
    public:
        Core(const glm::ivec2 &resolution, bool enableDebugging);
        ~Core();
        
        void setScene(std::unique_ptr<Scene> scene, const std::filesystem::path &path);
        
        void run();
        
        [[nodiscard]] Scene *getScene();
        [[nodiscard]] MainCamera *getCamera();
    
    protected:
        bool initGlfw(int openGlMajorVersion, int openGlMinorVersion);
        bool initImGui();
        void updateImgui();
        static void configureUiThemeColours(ImGuiStyle &style) ;
    
    protected:
        const glm::ivec2 mResolution { 1920, 1080 };
        const std::string_view mWindowTitle { "Game Engine" };
        
        GLFWimage mWindowIcon{};
        GLFWwindow *mWindow { nullptr };
        
        std::unique_ptr<Scene> mScene;
        Scene *mScenePointer;
        std::filesystem::path mScenePath;
        
        std::unique_ptr<ResourcePool> mResourcePool;
        std::unique_ptr<Renderer> mRenderer;
        std::unique_ptr<debug::Logger> mLogger;
        std::unique_ptr<ProfilerViewer> mProfilerViewer;
        std::unique_ptr<Profiler> mProfiler;
        std::unique_ptr<Serializer> mSerializer;
        
        ImGuiIO *mGuiIo { nullptr };
        bool mIsRunning { true };
        const unsigned int mMaxLoopCount { 10 };
        const bool mEnableDebugging { false };
        
        std::unique_ptr<MainCamera> mMainCamera;
        
        Editor mEditor;
        RootEventHandler mEventHandler;
        
    };
    
} // engine
