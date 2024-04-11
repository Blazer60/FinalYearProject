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
            const std::string meshPath = openFileDialog();
            if (const SharedMesh mesh = load::model<StandardVertex>(meshPath); !mesh->empty())
            {
                mMeshes = mesh;
                mMeshPath = meshPath;
            }
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceModelPayload))
            {
                const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                if (const SharedMesh mesh = load::model<StandardVertex>(path); !mesh->empty())
                {
                    mMeshes = mesh;
                    mMeshPath = path.string();
                }
            }
        }
    }

    void MeshRenderer::drawMaterialArray()
    {
        if (ui::seperatorTextButton(format::string("Materials (%)", mUberMaterials.size())))
        {
            addUMaterial(nullptr);
        }

        if (ImGui::BeginTable("Materials Table", 3))
        {
            const float textSizing = ImGui::CalcTextSize("00").x;
            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, textSizing);
            ImGui::TableSetupColumn("Contents", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Delete Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, ui::buttonSize().x);

            for (int i = 0; i < mUberMaterials.size();)
            {
                if (!drawMaterialElement(i))
                {
                    ++i;
                }
            }

            ImGui::EndTable();
        }
    }

    void MeshRenderer::drawMaterialElementColumn(const std::string& name, int index)
    {
        const bool selected = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);

        // todo: this most likely needs a different name.
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
            {
                containers::moveInPlace(mUberMaterials, *static_cast<int*>(payload->Data), index);
            }

            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceMaterialPayload))
            {
                const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                addUMaterial(load::material(path), index);
            }

            ImGui::EndDragDropTarget();
        }

        if (selected)
        {
            ui::draw(mUberMaterials[index]);
            ImGui::TreePop();
        }
    }

    bool MeshRenderer::drawMaterialElement(const int index)
    {
        const std::string name = mUberMaterials[index] != nullptr ? mUberMaterials[index]->name() : format::string("Empty##%", index);

        bool result = false;
        ImGui::PushID(name.c_str());
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.f), "%2i", index);
        }
        if (ImGui::TableNextColumn())
        {
            const auto &style = ImGui::GetStyle();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y);
            drawMaterialElementColumn(name, index);
        }
        if (ImGui::TableNextColumn())
        {
            result = ui::closeButton("Close innit");
            if (result)
                mUberMaterials.erase(mUberMaterials.begin() + index);
        }
        ImGui::PopID();

        return result;
    }

    void MeshRenderer::addUMaterial(std::shared_ptr<UberMaterial> material)
    {
        mUberMaterials.push_back(std::move(material));
    }

    void MeshRenderer::addUMaterial(const std::shared_ptr<UberMaterial> &material, const int index)
    {
        mUberMaterials[index] = material;
    }

    void MeshRenderer::onPreRender()
    {
        if (!mIsShowing || mMeshes->empty())
            return;

        auto submit = [this](const SubMesh &surface, const std::shared_ptr<UberMaterial> &material) {
            if (material != nullptr && !material->empty())
            {
                graphics::renderer->drawMesh(
                    surface,
                    getWorldTransform(),
                    material->getData());
            }
            else
            {
                graphics::renderer->drawMesh(
                    surface,
                    getWorldTransform(),
                    core->getDefaultLitMaterial()->getData());
            }
        };

        if (!mUberMaterials.empty())
        {
            for (int i = 0; i < mMeshes->size(); ++i)
            {
                const int meshIndex = glm::min(i, static_cast<int>(mMeshes->size() - 1));
                const int materialIndex = glm::min(i, static_cast<int>(mUberMaterials.size() - 1));
                submit(*(*mMeshes)[meshIndex], mUberMaterials[materialIndex]);
            }
        }
        else
        {
            // Fallback on the engine's default lit material just so that meshes are actually shown.
            for (int i = 0; i < mMeshes->size(); ++i)
            {
                graphics::renderer->drawMesh(
                    *(*mMeshes)[i],
                    getWorldTransform(),
                    core->getDefaultLitMaterial()->getData());
            }
        }
    }
}

