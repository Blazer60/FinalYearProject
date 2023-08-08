/**
 * @file Scene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Scene.h"
#include "imgui.h"
#include "WindowHelpers.h"

namespace engine
{
    void Scene::update()
    {
        onUpdate();
        
        for (auto &actor : mActors)
        {
            actor->OnUpdate();
            for (auto &component : actor->getComponents())
                component->update();
        }
    }

    void Scene::onFixedUpdate()
    {

    }

    void Scene::onUpdate()
    {

    }

    void Scene::onRender()
    {

    }

    void Scene::onImguiUpdate()
    {

    }

    void Scene::onImguiMenuUpdate()
    {

    }
    
    void Scene::imguiUpdate()
    {
        ImGui::Begin("Scene Hierarchy");
        if (ImGui::BeginListBox("##ActorHierarchyListBox", ImVec2(-FLT_MIN, -FLT_MIN)))
        {
            for (auto &actor : mActors)
            {
                const bool isSelected = (actor.get() == mSelectedActor);
                // All entries must have unique names.
                const std::string name = std::string(actor->getName()) + "##" + std::to_string(reinterpret_cast<int64_t>(&actor));
                if (ImGui::Selectable(name.c_str(), isSelected))
                {
                    mSelectedActor = actor.get();
                    MESSAGE("Selected Actor: " + name);
                }
                
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        ImGui::End();
        
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
        
        onImguiUpdate();
    }
    
}
