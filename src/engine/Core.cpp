/**
 * @file Core.cpp
 * @author Ryan Purse
 * @date 12/06/2023
 */


#include "Core.h"
#include "Scene.h"
#include "WindowHelpers.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Timers.h"
#include "Ui.h"
#include "RendererImGui.h"
#include "GraphicsState.h"
#include "EngineState.h"
#include "LoggerMacros.h"
#include "ImGuizmo.h"
#include "TextureLoader.h"
#include "ModelDestroyer.h"
#include "Profiler.h"
#include "ProfileTimer.h"
#include "GraphicsFunctions.h"

namespace engine
{
    Core::Core(const glm::ivec2 &resolution, bool enableDebugging)
        : mResolution(resolution), mEnableDebugging(enableDebugging)
    {
        mProfiler = std::make_unique<Profiler>();
        profiler = mProfiler.get();
        mLogger = std::make_unique<Logger>();
        logger = mLogger.get();
        mLogger->setOutputFlag(OutputSourceFlag_File | OutputSourceFlag_Queue);
        eventHandler = &mEventHandler;
        core = this;
        editor = &mEditor;
        
        window::setBufferSize(mResolution);
        
        if (!initGlfw(4, 6))
        {
            mIsRunning = false;
            LOG_MAJOR("Unable to initialise everything.");
            return;
        }
        
        if (glewInit() != GLEW_OK)
        {
            mIsRunning = false;
            LOG_MAJOR("Unable to initialise glew.");
            return;
        }
        
        mEditor.init();
        
        mRenderer = std::make_unique<Renderer>();
        if (!mRenderer->isOk)
        {
            mIsRunning = false;
            LOG_MAJOR("Could not load the renderer.");
            return;
        }
        
        graphics::renderer = mRenderer.get();
        
        if (mEnableDebugging)
        {
            bool success = Renderer::debugMessageCallback(forwardOpenGlCallback);
            if (!success)
                WARN("Unable to enable debugging. Check if the openGl version is greater than 4.3.");
            MESSAGE(Renderer::getVersion());
        }
        
        if (!initImGui())
        {
            mIsRunning = false;
            LOG_MAJOR("Could not load drawUi for an opengl context.");
            return;
        }
        
        mMainCamera = std::make_unique<MainCamera>(glm::vec3(0.f, 3.f, 21.f));
        
        mWindowIcon = load::windowIcon("../resources/textures/Icon.png");
        if (mWindowIcon.pixels != nullptr)
            glfwSetWindowIcon(mWindow, 1, &mWindowIcon);
        
        mStandardShader = std::make_shared<Shader>(
            "../resources/shaders/geometry/standard/Standard.vert",
            "../resources/shaders/geometry/standard/Standard.frag");
        
    }
    
    bool engine::Core::initGlfw(int openGlMajorVersion, int openGlMinorVersion)
    {
        if (!glfwInit())
            return false;
        
        glfwSetErrorCallback([](int errorCode, const char *description) {
            MESSAGE(description); // The program will bail out after this.
        });
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGlMajorVersion);  // Version of opengl you want to use
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGlMinorVersion);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        if (mEnableDebugging)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        
        mWindow = glfwCreateWindow(mResolution.x, mResolution.y, mWindowTitle.data(), nullptr, nullptr);
        if (!mWindow)
            return false;
        
        glfwMakeContextCurrent(mWindow);
        
        const int frameRate = 60;
        glfwSwapInterval(frameRate);
        
        glfwSetCursorPosCallback(mWindow, [](GLFWwindow *window, double xPos, double yPos) {
            eventHandler->updateMouseDelta(xPos, yPos);
        });
        
        return true;
    }
    
    bool Core::initImGui()
    {
        ImGui::CreateContext();
        
        mGuiIo = &ImGui::GetIO();
        (void)mGuiIo;
        mGuiIo->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        mGuiIo->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        mGuiIo->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        
        mGuiIo->Fonts->AddFontFromFileTTF("../resources/fonts/robotoMono/static/RobotoMono-Regular.ttf", 15.f);
        
        ImGui::StyleColorsDark();
        ImGuiStyle &style = ImGui::GetStyle();
        if (mGuiIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 5.f;
            style.Colors[ImGuiCol_WindowBg].w = 1.f;
        }
        
        style.FrameRounding = 5.f;
        style.GrabRounding = 5.f;
        
        style.WindowBorderSize = 0.f;
        style.ChildBorderSize = 0.f;
        style.PopupBorderSize = 0.f;
        
        
        configureUiThemeColours(style);
        
        if (!ImGui_ImplGlfw_InitForOpenGL(mWindow, true))
            return false;
        
        if (!ImGui_ImplOpenGL3_Init())
            return false;
        
        return true;
    }
    
    void Core::configureUiThemeColours(ImGuiStyle &style)
    {
        const ImVec4 background = ImVec4(0.16f, 0.16f, 0.17f, 1.f);
        const ImVec4 accentBackground = ImVec4(0.196f, 0.212f, 0.197f, 1.f);
        
        const ImVec4 primary        = ImVec4(0.2f, 0.2f, 0.21f, 1.f);
        const ImVec4 primaryHovered = ImVec4(0.3f, 0.3f, 0.32f, 1.f);
        const ImVec4 primaryActive  = ImVec4(0.4f, 0.4f, 0.48f, 1.f);
        
        const ImVec4 accent         = ImVec4(0.09f, 0.32f, 0.14f, 1.f);
        const ImVec4 accentHovered  = ImVec4(0.14f, 0.47f, 0.2f, 1.f);
        const ImVec4 accentActive   = ImVec4(0.18f, 0.62f, 0.27f, 1.f);
        
        style.Colors[ImGuiCol_Text]     = ImVec4(0.85f, 0.85f, 0.85f, 1.f);
        style.Colors[ImGuiCol_WindowBg] = background;
        style.Colors[ImGuiCol_ChildBg]  = background;
        style.Colors[ImGuiCol_PopupBg]  = accentBackground;
        
        style.Colors[ImGuiCol_Header]           = primary;
        style.Colors[ImGuiCol_HeaderHovered]    = primaryHovered;
        style.Colors[ImGuiCol_HeaderActive]     = primaryActive;
        
        style.Colors[ImGuiCol_FrameBg]          = primary;
        style.Colors[ImGuiCol_FrameBgHovered]   = primaryHovered;
        style.Colors[ImGuiCol_FrameBgActive]    = primaryActive;
        
        style.Colors[ImGuiCol_TitleBg] = background;
        style.Colors[ImGuiCol_TitleBgActive] = background;
        
        style.Colors[ImGuiCol_CheckMark]            = accentActive;
        style.Colors[ImGuiCol_SliderGrab]           = accentHovered;
        style.Colors[ImGuiCol_SliderGrabActive]     = accentActive;
        style.Colors[ImGuiCol_Button]               = accent;
        style.Colors[ImGuiCol_ButtonHovered]        = accentHovered;
        style.Colors[ImGuiCol_ButtonActive]         = accentActive;
        
        style.Colors[ImGuiCol_Separator] = primary;
        style.Colors[ImGuiCol_SeparatorHovered] = accentHovered;
        style.Colors[ImGuiCol_SeparatorActive] = accentActive;
        
        style.Colors[ImGuiCol_Tab]                  = primary;
        style.Colors[ImGuiCol_TabHovered]           = primaryHovered;
        style.Colors[ImGuiCol_TabActive]            = primary;
        style.Colors[ImGuiCol_TabUnfocused]         = background;
        style.Colors[ImGuiCol_TabUnfocusedActive]   = background;
        
        style.Colors[ImGuiCol_DockingPreview] = accentHovered;
        style.Colors[ImGuiCol_DragDropTarget] = accent;
    }
    
    Core::~Core()
    {
        destroy::windowIcon(mWindowIcon);
        mScene.reset();
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        glfwTerminate();
    }
    
    void Core::run()
    {
        if (mScene == nullptr)
            mIsRunning = false;
        
        double nextUpdateTick = 0.0;
        
        while (mIsRunning)
        {
            PROFILE_SCOPE_BEGIN(coreLoopTimer, "CPU Time");
            unsigned int loopAmount = 0;
            
            mEventHandler.beginFrame();
            glfwPollEvents();
            mIsRunning = !glfwWindowShouldClose(mWindow);
            
            PROFILE_SCOPE_BEGIN(fixedTimer, "Fixed Update");
            while (timers::getTicks<double>() > nextUpdateTick && loopAmount < mMaxLoopCount)
            {
                mScene->onFixedUpdate();
                
                
                nextUpdateTick += timers::fixedTime<double>();
                ++loopAmount;
            }
            PROFILE_SCOPE_END(fixedTimer);
            
            mScene->update();
            mMainCamera->update();
            mEditor.update();
            mScene->render();
            mRenderer->submit(mMainCamera->toSettings());
            mRenderer->render();
            updateImgui();
            mRenderer->clear();
            
            timers::update();
            PROFILE_SCOPE_END(coreLoopTimer);
            
            PROFILE_SCOPE_BEGIN(awaitVSync, "CPU Idle");
            glfwSwapBuffers(mWindow);
            PROFILE_SCOPE_END(awaitVSync);
            
            mProfiler->updateAndClear();
        }
    }
    
    void Core::updateImgui()
    {
        PROFILE_FUNC();
        graphics::pushDebugGroup("ImGUI Pass");
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(0);
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        
        mEventHandler.update();
        
        if (ImGui::BeginMainMenuBar())
        {
            mScene->onImguiMenuUpdate();
            const std::string text = "TPS: %.0f | Frame Rate: %.3f ms/frame (%.1f FPS)";
            ImGui::SetCursorPosX(
                ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
                - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text(text.c_str(), 1.f / timers::fixedTime<float>(), timers::deltaTime<float>() * 1000.f, ImGui::GetIO().Framerate);
            ImGui::EndMainMenuBar();
        }
        
        ImGui::Begin("Scene Settings");
        mScene->imguiUpdate();
        ImGui::End();
        
        ImGui::Begin("Renderer Settings");
        ui::draw(mMainCamera);
        graphics::displayShadowSettings();
        ImGui::End();
        
        ui::draw(mEditor);
        ui::draw(mProfiler);
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(mWindow, &display_w, &display_h);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (mGuiIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        
        graphics::popDebugGroup();
    }
    
    void Core::setScene(std::unique_ptr<Scene> scene)
    {
        mScene.reset();
        mScene = std::move(scene);
    }
    
    Scene *Core::getScene()
    {
        return mScene.get();
    }
    
    MainCamera *Core::getCamera()
    {
        return mMainCamera.get();
    }
    
    std::shared_ptr<Shader> &Core::getStandardShader()
    {
        return mStandardShader;
    }
}
