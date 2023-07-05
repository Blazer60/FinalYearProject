/**
 * @file Core.h
 * @author Ryan Purse
 * @date 12/06/2023
 */


#pragma once

#include "Pch.h"
#include "glfw3.h"
#include "imgui.h"
#include "TextureBufferObject.h"
#include "Renderer.h"

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
        
    protected:
        bool initGlfw(int openGlMajorVersion, int openGlMinorVersion);
        bool initImGui();
        
        void updateImgui();
        void updateImguiMenuViewports();
        void updateViewports();
        
    protected:
        const glm::ivec2 mResolution { 1920, 1080 };
        const std::string_view mWindowTitle { "Game Engine" };
        
        GLFWwindow *mWindow { nullptr };
        
        std::unique_ptr<Scene> mScene;
        std::unique_ptr<Renderer> mRenderer;
        
        ImGuiIO *mGuiIo { nullptr };
        bool mIsRunning { true };
        const unsigned int mMaxLoopCount { 10 };
        const bool mEnableDebugging { false };
        ViewportToggles mViewport;
    };
    
} // engine
