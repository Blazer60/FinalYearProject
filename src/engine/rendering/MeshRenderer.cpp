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

#include "ContainerAlgorithms.h"
#include "Ui.h"

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
            

            ImGui::BeginGroup();
            drawMaterialArray();
            ImGui::EndGroup();
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceMaterialPayload))
                {
                    const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                    mUberMaterials.push_back(load::material(path));
                }
                ImGui::EndDragDropTarget();
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

    void MeshRenderer::drawMaterialArray()
    {
        const auto name = format::string("Materials %/%", mUberMaterials.size(),  mMeshes->size());
        const auto originalCursorPos = ImGui::GetCursorPos();
        const auto PlusMarkposition = ImVec2(ImGui::GetContentRegionAvail().x - ui::resetButtonWidth(), originalCursorPos.y);

        ImGui::Text(name.c_str());
        ImGui::SetCursorPos(PlusMarkposition);

        const auto plusId = format::string(" + ##%", name);
        if (ImGui::Button(plusId.c_str()))
            mUberMaterials.push_back(nullptr);

        for (int i = 0; i < mUberMaterials.size();)
        {
            if (!drawMaterialElement(i))
                ++i;
        }
    }

    bool MeshRenderer::drawMaterialElement(int index)
    {
        const std::string name = mUberMaterials[index] != nullptr ? mUberMaterials[index]->name() : format::string("Empty##%", index);

        auto &style = ImGui::GetStyle();
        ImGui::BeginGroup();
        const auto originalCursorPos = ImGui::GetCursorPos();
        const auto xMarkposition = ImVec2(ImGui::GetContentRegionAvail().x - ui::resetButtonWidth(), originalCursorPos.y);

        ImGui::SetCursorPosY(originalCursorPos.y + style.FramePadding.y);
        ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), "%2i", index);
        ImGui::SameLine();
        ImGui::SetCursorPosY(originalCursorPos.y + style.FramePadding.y);
        ImGui::Selectable(name.c_str(), false, 0, ImVec2(ImGui::GetContentRegionAvail().x - ui::resetButtonWidth() - 3.f * style.FramePadding.x - 3.f * style.ItemSpacing.x, 0));

        constexpr auto arrayPayLoadId = "ARRAY_PAYLOAD_MATERIALS";
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload(arrayPayLoadId, &index, sizeof(int));
            ImGui::Text(name.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(arrayPayLoadId))
                containers::moveInPlace(mUberMaterials, *static_cast<int*>(payload->Data), index);

            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceMaterialPayload))
            {
                const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                mUberMaterials[index] = load::material(path);
            }

            ImGui::EndDragDropTarget();
        }

        const auto xMark = format::string(" X ##%", index);
        ImGui::SetCursorPos(xMarkposition);
        const bool result = ImGui::Button(xMark.c_str());
        if (result)
        {
            mUberMaterials.erase(mUberMaterials.begin() + index);
        }
        ImGui::EndGroup();
        return result;

    }

    void MeshRenderer::addMaterial(const std::shared_ptr<MaterialSubComponent> &material)
    {
        mMaterials.push_back(material);
    }

    void MeshRenderer::addUMaterial(std::shared_ptr<UberMaterial> material)
    {
        mUberMaterials.push_back(std::move(material));
    }

    void MeshRenderer::onPreRender()
    {
        if (mMeshes->empty() || mUberMaterials.empty())
            return;

        for (int i = 0; i < mMeshes->size(); ++i)
        {
            const int meshIndex = glm::min(i, static_cast<int>(mMeshes->size() - 1));
            const int materialIndex = glm::min(i, static_cast<int>(mUberMaterials.size() - 1));
            graphics::renderer->drawMesh(*(*mMeshes)[meshIndex], getWorldTransform(), mUberMaterials[materialIndex]->getData());
        }

        // if (mMaterials.empty() || mMeshes->empty())
        //     return;
        //
        // for (int i = 0; i < glm::max(mMaterials.size(), mMeshes->size()); ++i)
        // {
        //     const int meshIndex = glm::min(i, static_cast<int>(mMeshes->size() - 1));
        //     const int materialIndex = glm::min(i, static_cast<int>(mMaterials.size() - 1));
        //     graphics::renderer->drawMesh(*(*mMeshes)[meshIndex], mMaterials[materialIndex]->getMaterial(), getWorldTransform());
        // }
    }
}

