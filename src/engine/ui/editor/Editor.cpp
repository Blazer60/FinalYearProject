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
            actor->addComponent(Resource<MeshComponent>(model, material));
        });
        
        addComponentOption<DirectionalLight>("Directional Light", [](Ref<Actor> actor) {
            actor->addComponent(Resource<DirectionalLight>(
                glm::normalize(glm::vec3(1.f, 1.f, 1.f)),
                glm::vec3(0.93f, 0.93f, 0.95f),
                glm::ivec2(4096),
                4));
        });
        
        mViewport.init();
    }
    
    void Editor::onDrawUi()
    {
        ui::draw(mViewport);
        ui::draw(mLogWindow);
        drawSceneHierarchyPanel();
        drawActorDetails();
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
                if (ImGui::MenuItem("Actor"))
                {
                    mSelectedActor = core->getScene()->spawnActor<Actor>("Actor");
                    mSelectedActor->position = core->getCamera()->getEndOfBoomArmPosition();
                }
                if (ImGui::MenuItem("Cube"))
                    createDefaultShape("Cube", "../resources/models/defaultObjects/DefaultCube.glb");
                if (ImGui::MenuItem("Sphere"))
                    createDefaultShape("Sphere", "../resources/models/defaultObjects/DefaultSphere.glb");
                if (ImGui::MenuItem("Torus"))
                    createDefaultShape("Torus", "../resources/models/defaultObjects/DefaultTorus.glb");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        if (ImGui::BeginListBox("##ActorHierarchyListBox", ImVec2(-FLT_MIN, -FLT_MIN)))
        {
            for (Ref<Actor> actor : core->getScene()->getActors())
                drawSceneHierarchyForActor(actor);
            
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
    
    void Editor::createDefaultShape(const std::string& name, std::string_view path)
    {
        Ref<Actor> actor = core->getScene()->spawnActor<Actor>(name);
        actor->position = core->getCamera()->getEndOfBoomArmPosition();
        auto model = load::model<StandardVertex>(path);
        auto material = std::make_shared<StandardMaterial>();
        material->attachShader(core->getStandardShader());
        actor->addComponent(Resource<MeshComponent>(model, material));
        mSelectedActor = actor;
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
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                mSelectedActor = actor;
            
            for (Ref<Actor> child : actor->getChildren())
                drawSceneHierarchyForActor(child);
                
            ImGui::TreePop();
        }
    }
}
