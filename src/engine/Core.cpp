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

engine::Core::Core(const glm::ivec2 &resolution, bool enableDebugging)
    : mResolution(resolution), mEnableDebugging(enableDebugging)
{
    mLogger = std::make_unique<Logger>();
    logger = mLogger.get();
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
    
    glfwSetMouseButtonCallback(mWindow, [](GLFWwindow *window, int button, int action, int mods){
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, 0.0, 0.0);
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    });
    
    const int frameRate = 60;
    glfwSwapInterval(frameRate);
    
    return true;
}

bool engine::Core::initImGui()
{
    ImGui::CreateContext();
    
    mGuiIo = &ImGui::GetIO();
    (void)mGuiIo;
    mGuiIo->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    mGuiIo->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    mGuiIo->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    if (mGuiIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.f;
        style.Colors[ImGuiCol_WindowBg].w = 1.f;
    }
    
    if (!ImGui_ImplGlfw_InitForOpenGL(mWindow, true))
        return false;
    
    if (!ImGui_ImplOpenGL3_Init())
        return false;
    
    return true;
}

engine::Core::~Core()
{
    mScene.reset();
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    glfwTerminate();
}

void engine::Core::run()
{
    if (mScene == nullptr)
        mIsRunning = false;
    
    double nextUpdateTick = 0.0;
    
    while (mIsRunning)
    {
        unsigned int loopAmount = 0;
        
        while (timers::getTicks<double>() > nextUpdateTick && loopAmount < mMaxLoopCount)
        {
            mScene->onFixedUpdate();
            
            glfwPollEvents();
            mIsRunning = !glfwWindowShouldClose(mWindow);
            
            nextUpdateTick += timers::fixedTime<double>();
            ++loopAmount;
        }
        
        mScene->update();
        mMainCamera->update();
        mScene->onRender();
        mRenderer->submit(mMainCamera->toSettings());
        mRenderer->render();
        updateImgui();
        mRenderer->clear();
        
        glfwSwapBuffers(mWindow);
        timers::update();
    }
}

void engine::Core::updateImgui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    
    if (ImGui::BeginMainMenuBar())
    {
        mScene->onImguiMenuUpdate();
        updateImguiMenuViewports();
        const std::string text = "TPS: %.0f | Frame Rate: %.3f s/frame (%.1f FPS)";
        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
            - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
        ImGui::Text(text.c_str(), 1.f / timers::fixedTime<float>(), timers::deltaTime<float>(), ImGui::GetIO().Framerate);
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
}

void engine::Core::setScene(std::unique_ptr<Scene> scene)
{
    mScene.reset();
    mScene = std::move(scene);
}

void engine::Core::updateImguiMenuViewports()
{
    if (ImGui::BeginMenu("Viewports"))
    {
        if (ImGui::MenuItem("Show Position Buffer"))
            mViewportToggles.showPositionBuffer = true;
        if (ImGui::MenuItem("Show Normal Buffer"))
            mViewportToggles.showNormalBuffer = true;
        if (ImGui::MenuItem("Show Albedo Buffer"))
            mViewportToggles.showAlbedoBuffer = true;
        if (ImGui::MenuItem("Show Emissive Buffer"))
            mViewportToggles.showEmissiveBuffer = true;
        if (ImGui::MenuItem("Show Diffuse Buffer"))
            mViewportToggles.showDiffuseBuffer = true;
        if (ImGui::MenuItem("Show Depth Buffer"))
            mViewportToggles.showDepthBuffer = true;
        if (ImGui::MenuItem("Show Output Buffer"))
            mViewportToggles.showDeferredLightingBuffer = true;
        if (ImGui::MenuItem("Show Shadow Buffer"))
            mViewportToggles.showShadowBuffer = true;
        if (ImGui::MenuItem("Show Roughness Buffer"))
            mViewportToggles.showRoughnessBuffer = true;
        if (ImGui::MenuItem("Show Metallic Buffer"))
            mViewportToggles.showMetallicBuffer = true;
        if (ImGui::MenuItem("Show Primary Buffer"))
            mViewportToggles.showPrimaryBuffer = true;
        ImGui::EndMenu();
    }
}

void engine::Core::updateViewports()
{
    ui::showTextureBuffer("Albedo", mRenderer->getAlbedoBuffer(), &mViewportToggles.showAlbedoBuffer, false);
    ui::showTextureBuffer("Position", mRenderer->getPositionBuffer(), &mViewportToggles.showPositionBuffer, false);
    ui::showTextureBuffer("Normal", mRenderer->getNormalBuffer(), &mViewportToggles.showNormalBuffer, false);
    ui::showTextureBuffer("Emissive", mRenderer->getEmissiveBuffer(), &mViewportToggles.showEmissiveBuffer, false);
    ui::showTextureBuffer("Diffuse", mRenderer->getDiffuseBuffer(), &mViewportToggles.showDiffuseBuffer, false);
    ui::showTextureBuffer("Depth", mRenderer->getDepthBuffer(), &mViewportToggles.showDepthBuffer, false);
    ui::showTextureBuffer("Shadow", mRenderer->getShadowBuffer(), &mViewportToggles.showShadowBuffer, false);
    ui::showTextureBuffer("Roughness", mRenderer->getRoughnessBuffer(), &mViewportToggles.showRoughnessBuffer, false);
    ui::showTextureBuffer("Metallic", mRenderer->getMetallicBuffer(), &mViewportToggles.showMetallicBuffer, false);
    ui::showTextureBuffer("DeferredLighting", mRenderer->getDeferredLightingBuffer(), &mViewportToggles.showDeferredLightingBuffer, false);
    ui::showTextureBuffer("Primary", mRenderer->getPrimaryBuffer(), &mViewportToggles.showPrimaryBuffer, true);
}

engine::Scene *engine::Core::getScene()
{
    return mScene.get();
}

MainCamera *engine::Core::getCamera()
{
    return mMainCamera.get();
}
