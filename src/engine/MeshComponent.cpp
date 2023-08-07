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
    
    void MeshComponent::onUpdate()
    {
        // todo: This should be part of a separate loop dictated on when the renderer needs to render.
        graphics::renderer->drawMesh(mSharedMesh, mSharedMaterials, getWorldTransform());
    }
    
    void MeshComponent::onDrawUi()
    {
        ImGui::PushID("MeshComponentSetting");
        if (ImGui::CollapsingHeader("Mesh Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (auto &material : mSharedMaterials)
                ui::draw(material.get());
        }
        ImGui::PopID();
    }
}
