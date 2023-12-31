/**
 * @file MeshRenderer.cpp
 * @author Ryan Purse
 * @date 17/10/2023
 */


#include "MeshRenderer.h"
#include "EngineState.h"
#include "Actor.h"
#include "Core.h"
#include "FileExplorer.h"
#include "GraphicsState.h"
#include "ResourceFolder.h"
#include "FileLoader.h"

#include <utility>

namespace engine
{
    MeshRenderer::MeshRenderer(SharedMesh &&mesh, std::string path)
        : mMeshes(mesh), mMeshPath(std::move(path))
    {
    
    }
    
    void MeshRenderer::onDrawUi()
    {
        ImGui::PushID("MeshRendererSettings");
        if (ImGui::TreeNodeEx("Mesh Renderer Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);
            
            if (mMeshes->empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                ImGui::Text("Mesh serializeComponent does contain any meshes.");
                ImGui::PopStyleColor();
                drawMeshOptions();
            }
            else
            {
                ImGui::Checkbox("Show", &mIsShowing);
                
                drawMeshOptions();
                for (auto &material : mMaterials)
                    ui::draw(material.get());
                
                if (ImGui::Button("Add Material"))
                {
                    // todo: How do we add different types of materials? - Most likely references them off disk.
                    auto standardMaterial = std::make_shared<StandardMaterialSubComponent>();
                    standardMaterial->attachShader(
                        load::shader(
                            file::shaderPath() / "geometry/standard/Standard.vert",
                            file::shaderPath() / "geometry/standard/Standard.frag"));
                    mMaterials.push_back(standardMaterial);
                }
                if (!mMaterials.empty())
                {
                    ImGui::SameLine();
                    if (ImGui::Button("Delete last Material"))
                    {
                        mMaterials.erase(--mMaterials.end());
                    }
                }
            }
            
            
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    
    void MeshRenderer::drawMeshOptions()
    {
        if (ImGui::Button("Change Mesh"))
        {
            std::string meshPath = openFileDialog();
            SharedMesh mesh = load::model<StandardVertex>(meshPath);
            if (!mesh->empty())
            {
                mMeshes = mesh;
                mMeshPath = meshPath;
            }
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceModelPayload))
            {
                std::filesystem::path path = *reinterpret_cast<std::filesystem::path*>(payload->Data);
                SharedMesh mesh = load::model<StandardVertex>(path);
                if (!mesh->empty())
                {
                    mMeshes = mesh;
                    mMeshPath = path.string();
                }
            }
        }
    }
    
    void MeshRenderer::addMaterial(const std::shared_ptr<MaterialSubComponent> &material)
    {
        mMaterials.push_back(material);
    }
    
    void MeshRenderer::onPreRender()
    {
        if (mMaterials.empty() || mMeshes->empty())
            return;
        
        for (int i = 0; i < glm::max(mMaterials.size(), mMeshes->size()); ++i)
        {
            const int meshIndex = glm::min(i, static_cast<int>(mMeshes->size() - 1));
            const int materialIndex = glm::min(i, static_cast<int>(mMaterials.size() - 1));
            graphics::renderer->drawMesh(*(*mMeshes)[meshIndex], mMaterials[materialIndex]->getMaterial(), getWorldTransform());
        }
    }
}

