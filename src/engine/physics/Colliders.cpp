/**
 * @file Colliders.cpp
 * @author Ryan Purse
 * @date 06/12/2023
 */


#include "Colliders.h"

#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>

#include "Actor.h"
#include "AssimpLoader.h"
#include "FileExplorer.h"
#include "FileLoader.h"
#include "PhysicsConversions.h"
#include "ResourceFolder.h"
#include "physicsMesh.h"
#include "RigidBody.h"

namespace engine
{
    void Collider::onAwake()
    {
        if (auto rb = mActor->getComponent<RigidBody>(false); rb.isValid())
            rb->setupRigidBody(getCollider());
    }

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

    MeshCollider::MeshCollider(const std::filesystem::path &path)
    {
        initialiseBasedOnPath(path);
    }

    void MeshCollider::onBegin()
    {
        if (mMeshShape)
            mMeshShape->setLocalScaling(physics::cast(mActor->scale));
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
                initialiseBasedOnPath(meshPath);
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceModelPayload))
                {
                    const std::filesystem::path path = *static_cast<std::filesystem::path*>(payload->Data);
                    initialiseBasedOnPath(path);
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

    const SharedMesh& MeshCollider::getDebugMesh() const
    {
        return mDebugShape;
    }

    void MeshCollider::initialiseBasedOnPath(std::filesystem::path path)
    {
        if (path.empty())
            return;

        mPath = std::move(path);
        mMeshColliderBuffer = load::physicsMesh(mPath);
        mMeshShape = std::make_unique<btBvhTriangleMeshShape>(&mMeshColliderBuffer->vertexArray, true);
        mDebugShape = load::model<PositionVertex>(mPath);
    }
}
