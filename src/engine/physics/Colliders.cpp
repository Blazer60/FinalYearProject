/**
 * @file Colliders.cpp
 * @author Ryan Purse
 * @date 06/12/2023
 */


#include "Colliders.h"

#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>

#include "Actor.h"
#include "FileExplorer.h"
#include "FileLoader.h"
#include "PhysicsConversions.h"
#include "ResourceFolder.h"
#include "physicsMesh.h"

namespace engine
{
    BoxCollider::BoxCollider()
        : mBoxCollider(physics::cast(mHalfExtent))
    {

    }

    BoxCollider::BoxCollider(const glm::vec3& extent)
        : mHalfExtent(extent), mBoxCollider(physics::cast(mHalfExtent))
    {

    }

    void BoxCollider::onDrawUi()
    {
        ImGui::PushID("BoxCollider");
        if (ImGui::TreeNodeEx("Box Collider", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);

            ImGui::DragFloat3("Half Extent", glm::value_ptr(mHalfExtent));
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    btCollisionShape* BoxCollider::getCollider()
    {
        return &mBoxCollider;
    }

    glm::vec3 BoxCollider::getHalfExtent() const
    {
        return mHalfExtent;
    }

    SphereCollider::SphereCollider()
        : mSphereShape(mRadius)
    {
    }

    SphereCollider::SphereCollider(const float radius)
        : mRadius(radius), mSphereShape(mRadius)
    {
    }

    void SphereCollider::onDrawUi()
    {
        ImGui::PushID("SphereCollider");
        if (ImGui::TreeNodeEx("Sphere Collider", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);

            ImGui::DragFloat("Radius", &mRadius);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    btCollisionShape* SphereCollider::getCollider()
    {
        return &mSphereShape;
    }

    float SphereCollider::getRadius() const
    {
        return mRadius;
    }

    MeshCollider::MeshCollider()
    {

    }

    MeshCollider::MeshCollider(std::filesystem::path path)
        : mPath(std::move(path))
    {
        if (!mPath.empty())
        {
            mMeshColliderBuffer = load::physicsMesh(mPath);
            mMeshShape = std::make_unique<btBvhTriangleMeshShape>(&mMeshColliderBuffer->vertexArray, true);
        }
    }

    void MeshCollider::onDrawUi()
    {
        ImGui::PushID("Mesh Collider");
        if (ImGui::TreeNodeEx("Mesh Collider", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);

            if (ImGui::Button("Change Mesh"))
            {
                const std::string meshPath = openFileDialog();
                mPath = meshPath;
                mMeshColliderBuffer = load::physicsMesh(meshPath);
                mMeshShape = std::make_unique<btBvhTriangleMeshShape>(&mMeshColliderBuffer->vertexArray, true);
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceModelPayload))
                {
                    const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                    mPath = path;
                    mMeshColliderBuffer = load::physicsMesh(path);
                    mMeshShape = std::make_unique<btBvhTriangleMeshShape>(&mMeshColliderBuffer->vertexArray, true);
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();

    }

    btCollisionShape* MeshCollider::getCollider()
    {
        return mMeshShape.get();
    }
}
