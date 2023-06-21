/**
 * @file Scene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Scene.h"
#include "imgui.h"

void engine::Scene::onFixedUpdate()
{

}

void engine::Scene::onUpdate()
{

}

void engine::Scene::onRender()
{

}

void engine::Scene::onImguiUpdate()
{

}

void engine::Scene::onImguiMenuUpdate()
{

}

void engine::Scene::showTextureBuffer(
    const std::string &name, const TextureBufferObject &texture, bool *show)
{
    if (show && !*show)
        return;
    
    ImGui::PushID(name.c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    if (ImGui::Begin(name.c_str(), show))
    {
        ImVec2 regionSize = ImGui::GetContentRegionAvail();
        ImGui::Image(reinterpret_cast<void *>(texture.getName()), regionSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopID();
}
