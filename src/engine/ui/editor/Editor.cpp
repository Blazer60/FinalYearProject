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
        if (mSelectedActor != nullptr)
        {
            ui::draw(mSelectedActor);
            if (!mSelectedActor->getComponents().empty())
            {
                ImGui::SeparatorText("Components");
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
            for (auto &actor : core->getScene()->getActors())
            {
                const bool isSelected = (actor.get() == mSelectedActor);
                // All entries must have unique names.
                const std::string name = std::string(actor->getName()) + "##" + std::to_string(reinterpret_cast<int64_t>(&actor));
                if (ImGui::Selectable(name.c_str(), isSelected))
                {
                    mSelectedActor = actor.get();
                    MESSAGE("Selected Actor: %", name);
                }
                
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        ImGui::End();
    }
    
    Actor *Editor::getSelectedActor()
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
        auto *actor = core->getScene()->spawnActor<Actor>(name);
        actor->position = core->getCamera()->getEndOfBoomArmPosition();
        auto model = load::model<StandardVertex>(path);
        auto material = std::make_shared<StandardMaterial>();
        material->attachShader(core->getStandardShader());
        actor->addComponent(std::make_unique<MeshComponent>(model, material));
        mSelectedActor = actor;
    }
}
