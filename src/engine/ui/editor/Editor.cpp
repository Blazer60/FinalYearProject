/**
 * @file Editor.cpp
 * @author Ryan Purse
 * @date 08/08/2023
 */


#include "Editor.h"
#include "Core.h"
#include "Scene.h"
#include "AssimpLoader.h"
#include "MeshComponent.h"

namespace engine
{
    void Editor::init()
    {
        addComponentOption<MeshComponent>("Mesh Component", [](Ref<Actor> actor)
        {
            auto model = load::model<StandardVertex>("");
            auto material = std::make_shared<StandardMaterial>();
            material->attachShader(core->getStandardShader());
            actor->addComponent(makeResource<MeshComponent>(model, material));
        });
        
        addComponentOption<DirectionalLight>("Directional Light", [](Ref<Actor> actor) {
            actor->addComponent(makeResource<DirectionalLight>(
                glm::normalize(glm::vec3(1.f, 1.f, 1.f)),
                glm::vec3(0.93f, 0.93f, 0.95f),
                glm::ivec2(4096),
                4));
        });
        
        addMenuOption("Actor", []() {
            Ref<Actor> actor = core->getScene()->spawnActor<Actor>("Actor");
            actor->position = core->getCamera()->getEndOfBoomArmPosition();
            return actor;
        });
        
        addMenuOption("Cube",   []() { return Editor::createDefaultShape("Cube",    "../resources/models/defaultObjects/DefaultCube.glb"); });
        addMenuOption("Sphere", []() { return Editor::createDefaultShape("Sphere",  "../resources/models/defaultObjects/DefaultSphere.glb"); });
        addMenuOption("Torus",  []() { return Editor::createDefaultShape("Torus",   "../resources/models/defaultObjects/DefaultTorus.glb"); });
        
        mViewport.init();
    }
    
    void Editor::onDrawUi()
    {
        ui::draw(mViewport);
        ui::draw(mLogWindow);
        drawSceneHierarchyPanel();
        drawActorDetails();
        
        if (mMoveSourceActor != nullptr)
            moveActors();
    }
    
    void Editor::drawActorDetails()
    {
        ImGui::Begin("Details");
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
        ImGui::End();
    }
    
    void Editor::drawSceneHierarchyPanel()
    {
        ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_MenuBar);
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
                
                for (Ref<Actor> actor : core->getScene()->getActors())
                    drawSceneHierarchyForActor(actor);
                
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
    
    }
    
    bool Editor::isViewportHovered()
    {
        return mViewport.isHovered();
    }
    
    GLFWwindow *Editor::getViewportContext()
    {
        return mViewport.getViewportContext();
    }
    
    Ref<Actor> Editor::createDefaultShape(const std::string& name, std::string_view path)
    {
        Ref<Actor> actor = core->getScene()->spawnActor<Actor>(name);
        actor->position = core->getCamera()->getEndOfBoomArmPosition();
        auto model = load::model<StandardVertex>(path);
        auto material = std::make_shared<StandardMaterial>();
        material->attachShader(core->getStandardShader());
        actor->addComponent(makeResource<MeshComponent>(model, material));
        
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
    
    void Editor::drawSceneHierarchyForActor(Ref<Actor> &actor)
    {
        const std::string name = std::string(actor->getName()) + "##" + std::to_string(reinterpret_cast<int64_t>(&actor));
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
                mSelectedActor = actor;
            
            
            for (Ref<Actor> child : actor->getChildren())
                drawSceneHierarchyForActor(child);
                
            ImGui::TreePop();
        }
    }
    
    void Editor::moveActors()
    {
        if (mMoveDestinationActor == nullptr)
        {
            if (Actor* parent = mMoveSourceActor->getParent(); parent != nullptr)
            {
                if (mMoveDestinationActor != parent)
                    core->getScene()->addActor(parent->popActor(mMoveSourceActor));
            }
        }
        else if (Actor* parent = mMoveSourceActor->getParent(); parent != nullptr)
        {
            if (mMoveDestinationActor != parent)
                mMoveDestinationActor->addChildActor(parent->popActor(mMoveSourceActor));
        }
        else
            mMoveDestinationActor->addChildActor(mMoveSourceActor->getScene()->popActor(mMoveSourceActor));
        
        mMoveSourceActor = nullptr;
        mMoveDestinationActor = nullptr;
    }
    
    void Editor::addMenuOption(const std::string &name, const ActorDetails::CreateFunc &onCreate)
    {
        mMenuList.push_back(ActorDetails { name, onCreate });
    }
}
