/**
 * @file MeshComponent.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "MeshComponent.h"
#include "GraphicsState.h"

namespace engine
{
    MeshComponent::MeshComponent(SharedMesh sharedMesh, SharedMaterials sharedMaterials)
        : mSharedMesh(std::move(sharedMesh)), mSharedMaterials(std::move(sharedMaterials))
    {
    }
    
    void MeshComponent::onDrawUi()
    {
        ImGui::PushID("MeshComponentSetting");
        if (ImGui::CollapsingHeader("Mesh Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (mSharedMesh.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                ImGui::Text("Mesh component does contain any meshes.");
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::Checkbox("Show", &mShow);
                for (auto &material : mSharedMaterials)
                    ui::draw(material.get());
            }
        }
        ImGui::PopID();
    }
    
    void MeshComponent::onPreRender()
    {
        if (mShow)
            graphics::renderer->drawMesh(mSharedMesh, mSharedMaterials, getWorldTransform());
    }
}
