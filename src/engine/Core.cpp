/**
 * @file Core.cpp
 * @author Ryan Purse
 * @date 12/06/2023
 */


#include "Core.h"

#include <alext.h>
#include <AL/alext.h>

#include "Camera.h"
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
#include "ModelDestroyer.h"
#include "Profiler.h"
#include "ProfileTimer.h"
#include "GraphicsFunctions.h"
#include "FileLoader.h"
#include "ComponentSerializer.h"
#include "FileExplorer.h"
#include "Loader.h"

namespace engine
{
    Core::Core(const glm::ivec2 &resolution, const bool enableDebugging)
        : mResolution(resolution), mEnableDebugging(enableDebugging)
    {
        mProfiler = std::make_unique<Profiler>();
        profiler = mProfiler.get();
        
        mLogger = std::make_unique<debug::Logger>();
        debug::logger = mLogger.get();
        mLogger->setOutputFlag(debug::OutputSourceFlag_File | debug::OutputSourceFlag_Queue | debug::OutputSourceFlag_IoStream);
        eventHandler = &mEventHandler;
        core = this;

        mResourcePool = std::make_unique<ResourcePool>();
        resourcePool = mResourcePool.get();


        window::setBufferSize(mResolution);
        
        file::findResourceFolder();
        
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
        
        mEditor = std::make_unique<Editor>();
        editor = mEditor.get();
        mEditor->init();
        
        mRenderer = std::make_unique<Renderer>();

        graphics::renderer = mRenderer.get();
        
        if (mEnableDebugging)
        {
            bool success = Renderer::debugMessageCallback(forwardOpenGlCallback);
            if (!success)
                WARN("Unable to enable debugging. Check if the openGl version is greater than 4.3.");
            MESSAGE_VERBOSE(Renderer::getVersion());
        }
        
        if (!initImGui())
        {
            mIsRunning = false;
            LOG_MAJOR("Could not load imgui for an opengl context.");
            return;
        }
        
        mWindowIcon = load::windowIcon((file::texturePath() / "Icon.png").string());
        if (mWindowIcon.pixels != nullptr)
            glfwSetWindowIcon(mWindow, 1, &mWindowIcon);
        
        mDefaultLitMaterial = load::material(file::modelPath() / "defaultObjects/DefaultWhite.mpcy");

        initOpenAL();
        
        mSerializer = std::make_unique<Serializer>();
        serializer = mSerializer.get();
        attachComponentSerialization();

        mPhysics = std::make_unique<PhysicsCore>();
        physicsSystem = mPhysics.get();
    }
    
    bool Core::initGlfw(int openGlMajorVersion, int openGlMinorVersion)
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
        
        // Enable/disable V-sync. 1 = on, else off. NVIDIA honours multiple numbers whereas AMD ignores them.
        glfwSwapInterval(1);
        
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
        
        mGuiIo->Fonts->AddFontFromFileTTF((file::resourcePath() / "fonts/robotoMono/static/RobotoMono-Regular.ttf").string().c_str(), 15.f);
        
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

    void Core::initOpenAL()
    {
        mAudioDevice = alcOpenDevice(nullptr);
        if (mAudioDevice == nullptr)
        {
            WARN("Failed to open audio device");
            return;
        }

        const std::vector flags = { ALC_CONTEXT_FLAGS_EXT, ALC_CONTEXT_DEBUG_BIT_EXT };
        mAudioContext = alcCreateContext(mAudioDevice, flags.data());

        if (mAudioContext == nullptr)
        {
            WARN("Failed to create audio context");
            alcCloseDevice(mAudioDevice);
            return;
        }

        if (alcMakeContextCurrent(mAudioContext) == 0)
        {
            WARN("Failed to make audio context the current context");
            alcDestroyContext(mAudioContext);
            alcCloseDevice(mAudioDevice);
        }

        alDebugMessageCallbackEXT([](ALenum source, ALenum type, ALuint id, ALenum severity, ALsizei length, const ALchar *message, void *userParam) {
            switch (severity)
            {
                case AL_DEBUG_SEVERITY_HIGH_EXT:
                    LOG_MAJOR(message);
                    break;
                case AL_DEBUG_SEVERITY_MEDIUM_EXT:
                    LOG_MINOR(message);
                    break;
                case AL_DEBUG_SEVERITY_LOW_EXT:
                    WARN(message);
                    break;
                case AL_DEBUG_SEVERITY_NOTIFICATION_EXT:
                default:
                    MESSAGE(message);
            }

            // ERROR("%", message);
        }, nullptr);

        alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
    }

    void Core::configureUiThemeColours(ImGuiStyle &style)
    {
        constexpr ImVec4 background = ImVec4(0.16f, 0.16f, 0.17f, 1.f);
        constexpr ImVec4 accentBackground = ImVec4(0.196f, 0.212f, 0.197f, 1.f);

        constexpr ImVec4 primary        = ImVec4(0.2f, 0.2f, 0.21f, 1.f);
        constexpr ImVec4 primaryHovered = ImVec4(0.3f, 0.3f, 0.32f, 1.f);
        constexpr ImVec4 primaryActive  = ImVec4(0.4f, 0.4f, 0.48f, 1.f);

        constexpr ImVec4 accent         = ImVec4(0.09f, 0.32f, 0.14f, 1.f);
        constexpr ImVec4 accentHovered  = ImVec4(0.14f, 0.47f, 0.2f, 1.f);
        constexpr ImVec4 accentActive   = ImVec4(0.18f, 0.62f, 0.27f, 1.f);
        
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

        style.Colors[ImGuiCol_TableRowBg] = background;
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.17f, 0.17f, 0.17f, 1.f);
    }
    
    Core::~Core()
    {
        // Should be calling scene cleanup.
        mScene.reset();

        // We don't clean between scene loads since we want to cache as much as possible between them.
        mResourcePool->clean();
        // An asset may be held by one or more engine handles. This is the last place to save before going out of scope.
        mResourcePool->saveAllAssets();

        destroy::windowIcon(mWindowIcon);

        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        glfwTerminate();

        alcMakeContextCurrent(nullptr);
        alcDestroyContext(mAudioContext);
        alcCloseDevice(mAudioDevice);
    }
    
    void Core::run()
    {
        if (mScene == nullptr)
            setScene(std::make_unique<Scene>(), "");

        double nextUpdateTick = timers::getTicks<double>();
        timers::update();  // To register a valid time into the system.
        timers::update();
        
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
                // We want the other variables to update to avoid 'catch-up' between play mode and edit mode.
                if (mIsInPlayMode)
                {
                    mPhysics->realignPhysicsObjects();
                    mPhysics->dynamicsWorld->stepSimulation(timers::fixedTime<float>(), 1.f, timers::fixedTime<float>());
                    mPhysics->resolveCollisoinCallbacks();
                    mPhysics->realignWorldObjects();
                    mScene->fixedUpdate();
                }

                nextUpdateTick += timers::fixedTime<double>();
                ++loopAmount;
            }
            PROFILE_SCOPE_END(fixedTimer);

            mScene->update();
            mEditor->update();

            mResourcePool->update();  // Calls material onPreRender() function.
            mEditor->preRender();
            mScene->preRender();
            mPhysics->renderDebugShapes();
            mRenderer->render();
            updateImgui();
            mRenderer->clear();
            
            timers::update();
            mResourcePool->clean();
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
        
        Renderer::rendererGuiNewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        
        mEventHandler.update();
        ui::draw(mEditor);

        ImGui::Render();

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
    
    void Core::setScene(std::unique_ptr<Scene> scene, const std::filesystem::path &path)
    {
        if (!(path.empty() || path.string() == tempFilePath))
        {
            mScenePath = path;
            mSceneName = file::makeRelativeToResourcePath(path).string();
        }

        mScene.reset();
        mScene = std::move(scene);
        mScenePointer = mScene.get();
        mPhysics->clearContainers();
        editor->relinkSelectedActor();
    }
    
    Scene *Core::getScene() const
    {
        return mScenePointer;
    }
    
    btDiscreteDynamicsWorld* Core::getPhysicsWorld() const
    {
        return mPhysics->dynamicsWorld.get();
    }

    const std::shared_ptr<UberMaterial>& Core::getDefaultLitMaterial() const
    {
        return mDefaultLitMaterial;
    }

    std::string Core::getSceneName()
    {
        return mSceneName;
    }

    std::filesystem::path Core::getScenePath() const
    {
        return mScenePath;
    }

    bool Core::isInPlayMode() const
    {
        return mIsInPlayMode;
    }

    void Core::setScenePath(std::filesystem::path path)
    {
        mScenePath = std::move(path);
    }

    void Core::beginPlay()
    {
        serialize::scene(tempFilePath, getScene());
        setScene(load::scene(tempFilePath), tempFilePath);
        mIsInPlayMode = true;
        mEventHandler.updateUserEvents = true;
    }

    void Core::endPlay()
    {
        setScene(load::scene(tempFilePath), mScenePath);
        mIsInPlayMode = false;
        mEventHandler.updateUserEvents = false;
    }
}
