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

namespace engine
{
    class Scene;
    
    struct ViewportToggles
    {
        bool showPositionBuffer  { true };
        bool showAlbedoBuffer    { true };
        bool showNormalBuffer    { true };
        bool showEmissiveBuffer  { true };
        bool showDeferredLightingBuffer    { true };
        bool showDiffuseBuffer   { true };
        bool showDepthBuffer     { true };
        bool showShadowBuffer    { true };
        bool showRoughnessBuffer { true };
        bool showMetallicBuffer  { true };
        bool showPrimaryBuffer { true };
    };

/**
 * @author Ryan Purse
 * @date 12/06/2023
 */
    class Core
    {
    public:
        Core(const glm::ivec2 &resolution, bool enableDebugging);
        ~Core();
        
        void setScene(std::unique_ptr<Scene> scene);
        
        void run();
        
        [[nodiscard]] Scene *getScene();
        [[nodiscard]] MainCamera *getCamera();
        
        std::shared_ptr<Shader> &getStandardShader();
        
    protected:
        bool initGlfw(int openGlMajorVersion, int openGlMinorVersion);
        bool initImGui();
        
        void updateImgui();
        void updateImguiMenuViewports();
        void updateViewports();
        
    protected:
        static void configureUiThemeColours(ImGuiStyle &style) ;
        
        const glm::ivec2 mResolution { 1920, 1080 };
        const std::string_view mWindowTitle { "Game Engine" };
        
        GLFWimage mWindowIcon;
        GLFWwindow *mWindow { nullptr };
        
        std::unique_ptr<Scene> mScene;
        std::unique_ptr<Renderer> mRenderer;
        std::unique_ptr<Logger> mLogger;
        
        ImGuiIO *mGuiIo { nullptr };
        bool mIsRunning { true };
        const unsigned int mMaxLoopCount { 10 };
        const bool mEnableDebugging { false };
        ViewportToggles mViewportToggles;
        
        std::unique_ptr<MainCamera> mMainCamera;
        
        Editor mEditor;
        RootEventHandler mEventHandler;
        
        std::shared_ptr<Shader> mStandardShader;
    };
    
} // engine
