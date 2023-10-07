/**
 * @file MeshComponent.cpp
 * @author Ryan Purse
 * @date 07/08/2023
 */


#include "MeshComponent.h"
#include "GraphicsState.h"
#include "Actor.h"
#include "FileExplorer.h"
#include "AssimpLoader.h"
#include "Core.h"

namespace engine
{
    MeshComponent::MeshComponent(SharedMesh sharedMesh, SharedMaterials sharedMaterials)
        : mSharedMesh(std::move(sharedMesh)), mSharedMaterials(std::move(sharedMaterials))
    {
    }
    
    void MeshComponent::onDrawUi()
    {
        ImGui::PushID("MeshComponentSetting");
        if (ImGui::TreeNodeEx("Mesh Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);
            
            if (mSharedMesh.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                ImGui::Text("Mesh component does contain any meshes.");
                ImGui::PopStyleColor();
                drawMeshOptions();
            }
            else
            {
                ImGui::Checkbox("Show", &mShow);
                drawMeshOptions();
                for (auto &material : mSharedMaterials)
                    ui::draw(material.get());
                
                if (ImGui::Button("Add Material"))
                {
                    auto material = std::make_shared<StandardMaterial>();
                    material->attachShader(engine::core->getStandardShader());
                    mSharedMaterials.push_back(material);
                }
            }
            
            
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    
    void MeshComponent::onPreRender()
    {
        if (mShow)
            graphics::renderer->drawMesh(mSharedMesh, mSharedMaterials, getWorldTransform());
    }
    
    void MeshComponent::drawMeshOptions()
    {
        if (ImGui::Button("Change Mesh"))
        {
            std::string meshPath = openFileExplorer();
            SharedMesh mesh = load::model<StandardVertex>(meshPath);
            if (!mesh.empty())
            {
                mSharedMesh = mesh;
            }
        }
    }
}
