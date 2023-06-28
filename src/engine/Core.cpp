/**
 * @file Core.cpp
 * @author Ryan Purse
 * @date 12/06/2023
 */


#include "Core.h"
#include "Scene.h"
#include "Renderer.h"
#include "WindowHelpers.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Timers.h"
#include "UiHelpers.h"

engine::Core::Core(const glm::ivec2 &resolution, bool enableDebugging)
    : mResolution(resolution), mEnableDebugging(enableDebugging)
{
    window::setBufferSize(mResolution);
    
    if (!(initGlfw(4, 6) && renderer::init() && initImGui()))
    {
        mIsRunning = false;
        debug::log("Unable to initialise everything.", debug::severity::Warning);
        return;
    }
    
    if (mEnableDebugging)
    {
        bool success = renderer::debugMessageCallback(debug::openglCallBack);
        if (!success)
        {
            debug::log("Unable to enable debugging. Check if the openGl version is greater than 4.3.",
                       debug::severity::Warning);
        }
    }
}

bool engine::Core::initGlfw(int openGlMajorVersion, int openGlMinorVersion)
{
    if (!glfwInit())
        return false;
    
    glfwSetErrorCallback([](int errorCode, const char *description) {
        debug::log(description);  // The program will bail out after this.
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
        
        mScene->onUpdate();
        mScene->onRender();
        renderer::render();
        updateImgui();
        
        glfwSwapBuffers(mWindow);
        timers::update();
    }
}

void engine::Core::updateImgui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
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
    mScene->onImguiUpdate();
    ImGui::End();
    updateViewports();
    
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
            mViewport.showPositionBuffer = true;
        if (ImGui::MenuItem("Show Normal Buffer"))
            mViewport.showNormalBuffer = true;
        if (ImGui::MenuItem("Show Albedo Buffer"))
            mViewport.showAlbedoBuffer = true;
        if (ImGui::MenuItem("Show Emissive Buffer"))
            mViewport.showEmissiveBuffer = true;
        if (ImGui::MenuItem("Show Diffuse Buffer"))
            mViewport.showDiffuseBuffer = true;
        if (ImGui::MenuItem("Show Specular Buffer"))
            mViewport.showSpecularBuffer = true;
        if (ImGui::MenuItem("Show Depth Buffer"))
            mViewport.showDepthBuffer = true;
        if (ImGui::MenuItem("Show Output Buffer"))
            mViewport.showOutputBuffer = true;
        if (ImGui::MenuItem("Show Shadow Buffer"))
            mViewport.showShadowBuffer = true;
        ImGui::EndMenu();
    }
}

void engine::Core::updateViewports()
{
    ui::showTextureBuffer("Albedo",     renderer::getAlbedoBuffer(),    &mViewport.showAlbedoBuffer,    false);
    ui::showTextureBuffer("Position",   renderer::getPositionBuffer(),  &mViewport.showPositionBuffer,  false);
    ui::showTextureBuffer("Normal",     renderer::getNormalBuffer(),    &mViewport.showNormalBuffer,    false);
    ui::showTextureBuffer("Emissive",   renderer::getEmissiveBuffer(),  &mViewport.showEmissiveBuffer,  false);
    ui::showTextureBuffer("Diffuse",    renderer::getDiffuseBuffer(),   &mViewport.showDiffuseBuffer,   false);
    ui::showTextureBuffer("Specular",   renderer::getSpecularBuffer(),  &mViewport.showSpecularBuffer,  false);
    ui::showTextureBuffer("Depth",      renderer::getDepthBuffer(),     &mViewport.showDepthBuffer,     false);
    ui::showTextureBuffer("Shadow",     renderer::getShadowBuffer(),    &mViewport.showShadowBuffer,    false);
    ui::showTextureBuffer("Output",     renderer::getOutputBuffer(),    &mViewport.showOutputBuffer,    true);
}
