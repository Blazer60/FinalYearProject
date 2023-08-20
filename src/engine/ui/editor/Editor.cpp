/**
 * @file Editor.cpp
 * @author Ryan Purse
 * @date 08/08/2023
 */


#include "Editor.h"
#include "Core.h"
#include "Scene.h"

namespace engine
{
    void Editor::init()
    {
        mViewport.init();
    }
    
    void Editor::onDrawUi()
    {
        // We do this here since the mouse wheel isn't registered until we begin drawing elements.
        ImGuiIO &io = ImGui::GetIO();
        mMouseWheel = io.MouseWheel;
        
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
        ImGui::Begin("Scene Hierarchy");
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
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            onMouseClicked.broadcast(ImGuiMouseButton_Right, true);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            onMouseClicked.broadcast(ImGuiMouseButton_Right, false);
        
        for (ImGuiKey key = (ImGuiKey)0; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
        {
            if (ImGui::IsKeyPressed(key))
                onKeyPressed.broadcast(key, true);
            else if (ImGui::IsKeyReleased(key))
                onKeyPressed.broadcast(key, false);
        }
    }
    
    bool Editor::isViewportFocused()
    {
        return mViewport.isFocused();
    }
    
    float Editor::getMouseWheel()
    {
        return mMouseWheel;
    }
}
