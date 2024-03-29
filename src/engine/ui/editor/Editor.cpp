/**
 * @file Editor.cpp
 * @author Ryan Purse
 * @date 08/08/2023
 */


#include "Editor.h"
#include "Core.h"
#include "Scene.h"
#include "ProfileTimer.h"
#include "Lighting.h"
#include "EngineState.h"
#include "MeshRenderer.h"
#include "SoundComponent.h"
#include <FileLoader.h>
#include "Colliders.h"
#include "FileExplorer.h"
#include "Ui.h"
#include "RigidBody.h"
#include "Camera.h"

namespace engine
{
    Editor::~Editor()
    {
        eventHandler->editor.onDeleteActor.unSubscribe(mDeleteActorToken);
    }

    void Editor::init()
    {
        addComponentOption<DirectionalLight>("Directional Light", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<DirectionalLight>(
                glm::normalize(glm::vec3(1.f, 1.f, 1.f)),
                glm::vec3(0.93f, 0.93f, 0.95f),
                glm::ivec2(4096),
                4));
        });
        
        addComponentOption<PointLight>("Point Light", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<PointLight>());
        });

        addComponentOption<Spotlight>("Spot Light", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<Spotlight>());
        });
        
        addComponentOption<DistantLightProbe>("Distant Light Probe", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<DistantLightProbe>(glm::ivec2(512)));
        });
        
        addComponentOption<MeshRenderer>("Mesh Renderer", [](Ref<Actor> actor) {
            const auto path = file::modelPath() / "defaultObjects/DefaultCube.glb";
            actor->addComponent(load::meshRenderer<StandardVertex>(path));
        });

        addComponentOption<SoundComponent>("Sound", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<SoundComponent>());
        });

        addComponentOption<BoxCollider>("Box Collider", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<BoxCollider>());
        });

        addComponentOption<SphereCollider>("Sphere Collider", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<SphereCollider>());
        });

        addComponentOption<RigidBody>("Rigid Body", [](Ref<Actor> acotr) {
            acotr->addComponent(makeResource<RigidBody>());
        });

        addComponentOption<MeshCollider>("Mesh Collider", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<MeshCollider>());
        });

        addComponentOption<Camera>("Camera", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<Camera>());
        });

        addMenuOption("Actor", [this]() {
            Ref<Actor> actor = core->getScene()->spawnActor<Actor>("Actor");
            actor->position = mViewport.getCamera()->getEndOfBoomArmPosition();
            return actor;
        });
        
        addMenuOption("Cube",   [this]() { return createDefaultShape("Cube",   (file::modelPath() / "defaultObjects/DefaultCube.glb").string()); });
        addMenuOption("Sphere", [this]() { return createDefaultShape("Sphere", (file::modelPath() / "defaultObjects/DefaultSphere.glb").string()); });
        addMenuOption("Torus",  [this]() { return createDefaultShape("Torus",  (file::modelPath() / "defaultObjects/DefaultTorus.glb").string()); });
        
        mViewport.init();

        mDeleteActorToken = eventHandler->editor.onDeleteActor.subscribe([this] {
            if (mSelectedActor.isValid())
                mSelectedActor->markForDeath();
        });
    }
    
    void Editor::onDrawUi()
    {
        PROFILE_FUNC();
        drawMenuBar();
        ui::draw(mViewport);
        ui::draw(mLogWindow);
        ui::draw(mResourceFolder);
        ui::draw(mProfilerViewer);
        drawSceneSettings();
        drawSceneHierarchyPanel();
        drawDetailsPanel();

        if (mMoveSourceActor != nullptr)
            moveActors();
    }
    
    void Editor::drawActorDetails()
    {
        if (mSelectedActor.isValid())
        {
            ui::draw(mSelectedActor.get());
            ImGui::SeparatorText("Components");
            drawAddComponentCombo();
            if (!mSelectedActor->getComponents().empty())
            {
                for (auto &component : mSelectedActor->getComponents())
                    ui::draw(component.get());
            }
        }
    }
    
    void Editor::drawSceneHierarchyPanel()
    {
        PROFILE_FUNC();
        if (!mShowSceneHierarchy)
            return;

        ImGui::Begin("Scene Hierarchy", &mShowSceneHierarchy, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Add"))
            {
                for (const auto &menuItem : mMenuList)
                {
                    const std::string name = menuItem.previewName + "##" + std::to_string(reinterpret_cast<uint64_t>(&menuItem));
                    if (ImGui::MenuItem(name.c_str()))
                        mSelectedActor = menuItem.onCreate();
                }
                ImGui::EndMenu();
            }
            if (mSelectedActor.isValid() && ImGui::Button("Deselect"))
                mSelectedActor = Ref<Actor>();
            ImGui::EndMenuBar();
        }
        
        if (ImGui::BeginListBox("##ActorHierarchyListBox", ImVec2(-FLT_MIN, -FLT_MIN)))
        {
            if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ActorHierarchy"))
                    {
                        Ref<Actor> payloadActor = *reinterpret_cast<Ref<Actor>*>(payload->Data);
                        
                        mMoveSourceActor = payloadActor.get();
                        mMoveDestinationActor = nullptr;
                    }
                    
                    ImGui::EndDragDropTarget();
                }
                
                for (const Ref<Actor> actor : core->getScene()->getActors())
                {
                    if (actor->getParent() == nullptr)
                        drawSceneHierarchyForActor(actor);
                }

                ImGui::TreePop();
            }

            ImGui::EndListBox();
        }
        ImGui::End();
    }
    
    Ref<Actor> Editor::getSelectedActor()
    {
        return mSelectedActor;
    }
    
    void Editor::update()
    {
        for (auto &callback : mOnUpdate)
            callback();
        
        mOnUpdate.clear();
        mViewport.update();
    }

    void Editor::preRender()
    {
        mViewport.preRender();
    }

    bool Editor::isViewportHovered()
    {
        return mViewport.isHovered();
    }

    bool Editor::isDebugOverlayOn() const
    {
        return mViewport.isDebugViewOn();
    }

    bool Editor::isUsingPlayModeCamera() const
    {
        return mViewport.isUsingPlayModeCamera();
    }

    GLFWwindow *Editor::getViewportContext()
    {
        return mViewport.getViewportContext();
    }
    
    Ref<Actor> Editor::createDefaultShape(const std::string& name, std::string_view path)
    {
        Ref<Actor> actor = core->getScene()->spawnActor<Actor>(name);
        actor->position = mViewport.getCamera()->getEndOfBoomArmPosition();
        auto model = actor->addComponent(load::meshRenderer<StandardVertex>(path));
        return actor;
    }
    
    void Editor::drawAddComponentCombo()
    {
        if (ImGui::BeginCombo("Add Component", nullptr, ImGuiComboFlags_NoPreview))
        {
            for (auto &componentDetails : mComponentList)
            {
                // Don't show this option if it already has it.
                if (componentDetails->hasThisComponent(mSelectedActor))
                    continue;
                
                std::string name = componentDetails->previewName + "##" + std::to_string(reinterpret_cast<int64_t>(&componentDetails));
                if (ImGui::Selectable(name.c_str()))
                    componentDetails->onCreate(mSelectedActor);
            }
            
            ImGui::EndCombo();
        }
    }

    void Editor::setSelectedActor(Ref<Actor> actor)
    {
        mSelectedActor = actor;
        mSelectedActorId = actor->getId();
        mSelectedType = selectedType::Actor;
    }

    void Editor::drawSceneHierarchyForActor(Ref<Actor> actor)
    {
        const std::string name = std::string(actor->getName()) + "##" + std::to_string(actor->getId());
        const bool isSelected = (actor.get() == mSelectedActor.get());
        int flags = actor->getChildren().empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen;
        if (isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;
        
        if (ImGui::TreeNodeEx(name.c_str(), flags | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth))
        {
            if (ImGui::BeginDragDropSource() && isSelected)
            {
                ImGui::SetDragDropPayload("ActorHierarchy", &actor, sizeof(Ref<Actor>));
                
                ImGui::Text("%s", actor->getName().data());
                
                ImGui::EndDragDropSource();
            }
            
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ActorHierarchy"))
                {
                    Ref<Actor> payloadActor = *reinterpret_cast<Ref<Actor>*>(payload->Data);
                    
                    mMoveSourceActor = payloadActor.get();
                    mMoveDestinationActor = actor.get();
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                setSelectedActor(actor);

            
            for (UUID childId : actor->getChildren())
                drawSceneHierarchyForActor(actor->getScene()->getActor(childId));
                
            ImGui::TreePop();
        }
    }

    void Editor::drawDetailsPanel()
    {
        PROFILE_FUNC();
        if (!mShowDetailsPanel)
            return;

        ImGui::Begin("Details", &mShowDetailsPanel);
        switch (mSelectedType)
        {
            case selectedType::Actor:
                drawActorDetails();
                break;
            case selectedType::Material:
                if (mUberMaterial != nullptr)
                    ui::draw(mUberMaterial);
                break;
            case selectedType::MaterialLayer:
                if (mUberLayer != nullptr)
                    ui::draw(mUberLayer);
                break;
        }
        ImGui::End();
    }

    void Editor::moveActors()
    {
        auto actorToMove = core->getScene()->getActor(mMoveSourceActor->getId());
        if (auto *const parent = actorToMove->getParent(); parent != nullptr)
            parent->removeChildActor(mMoveSourceActor);

        if (mMoveDestinationActor != nullptr)
            mMoveDestinationActor->addChildActor(actorToMove);

        mMoveSourceActor = nullptr;
        mMoveDestinationActor = nullptr;
    }
    
    void Editor::addMenuOption(const std::string &name, const ActorDetails::CreateFunc &onCreate)
    {
        mMenuList.push_back(ActorDetails { name, onCreate });
    }

    void Editor::drawFileMenuDropDown()
    {
        if (ImGui::BeginMenu("File", !core->isInPlayMode()))
        {
            if (ImGui::MenuItem("Save"))
            {
                const std::filesystem::path path = core->getScenePath();
                if (path.empty())
                    WARN("No Scene is loaded.");
                serialize::scene(path, core->getScene());
            }
            if (ImGui::MenuItem("Save as"))
            {
                if (const std::string scenePath = saveFileDialog(); !scenePath.empty())
                {
                    core->setScenePath(scenePath);
                    serialize::scene(scenePath, core->getScene());
                }
            }
            if (ImGui::MenuItem("Load"))
            {
                if (const auto scenePath = openFileDialog(); !scenePath.empty())
                    core->setScene(load::scene(scenePath), scenePath);
            }
            ImGui::EndMenu();
        }
    }

    void Editor::drawWindowDropDown()
    {
        if (ImGui::BeginMenu("Window"))
        {
            mLogWindow.isShowing        |= ImGui::MenuItem("Log Window");
            mViewport.isShowing         |= ImGui::MenuItem("Viewport");
            mResourceFolder.isShowing   |= ImGui::MenuItem("Resources");
            mProfilerViewer.isShowing   |= ImGui::MenuItem("Profiler");
            mShowSceneHierarchy         |= ImGui::MenuItem("Scene Hieararchy");
            mShowDetailsPanel           |= ImGui::MenuItem("Details Panel");
            mShowSceneSettings          |= ImGui::MenuItem("Show Scene Settings");

            mViewport.getCamera()->showCameraSettings |= ImGui::MenuItem("Show Camera Settings");
            ImGui::EndMenu();
        }
    }

    void Editor::drawMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            drawFileMenuDropDown();
            drawWindowDropDown();

            const std::string text = "TPS: %.0f | Frame Rate: %.3fms/frame (%.0fFPS)";
            ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
                - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);

            ImGui::Text(text.c_str(), 1.f / timers::fixedTime<float>(),
                timers::deltaTime<float>() * 1000.f, 1.f / timers::deltaTime<float>());
            ImGui::EndMainMenuBar();
        }


    }

    void Editor::addUpdateAction(const std::function<void()> &callback)
    {
        mOnUpdate.push_back(callback);
    }

    void Editor::relinkSelectedActor()
    {
        mSelectedActor = core->getScene()->getActor(mSelectedActorId, false);
    }

    void Editor::setUberLayer(std::shared_ptr<UberLayer> layer)
    {
        mUberLayer = std::move(layer);
        mSelectedType = selectedType::MaterialLayer;
    }

    void Editor::setUberMaterial(std::shared_ptr<UberMaterial> material)
    {
        mUberMaterial = std::move(material);
        mSelectedType = selectedType::Material;
    }

    void Editor::drawSceneSettings()
    {
        if (!mShowSceneSettings)
            return;

        ImGui::Begin("Scene Settings", &mShowSceneSettings);
        ui::draw(core->getScene());
        ImGui::End();
    }

    void Editor::createModel(const std::filesystem::path &path)
    {
        Ref<Actor> actor = core->getScene()->spawnActor<engine::Actor>(path.stem().string());
        actor->position = mViewport.getCamera()->getEndOfBoomArmPosition();
        Ref<MeshRenderer> meshRenderer = actor->addComponent(load::meshRenderer<StandardVertex>(path));

        setSelectedActor(actor);
    }
}
