/**
 * @file Colliders.cpp
 * @author Ryan Purse
 * @date 06/12/2023
 */


#include "Colliders.h"

#include "Actor.h"
#include "PhysicsConversions.h"

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
}
