/**
 * @file RigidBody.cpp
 * @author Ryan Purse
 * @date 07/12/2023
 */

#include "RigidBody.h"

#include <LinearMath/btDefaultMotionState.h>

#include "Actor.h"
#include "Colliders.h"
#include "Core.h"
#include "EngineState.h"
#include "PhysicsConversions.h"

namespace engine
{
    RigidBody::RigidBody(const float mass)
        : mMass(mass)
    {
    }

    RigidBody::~RigidBody()
    {
        if (mRigidBody)
            core->getPhysicsWorld()->removeRigidBody(mRigidBody.get());
    }

    void RigidBody::onBegin()
    {
        if (!core->isInPlayMode())  // Don't do anything in edit mode.
            return;

        Ref<Collider> collider = mActor->getComponent<Collider>();
        if (!collider.isValid())
        {
            WARN("A Rigid Body is attached to an actor but no collider is present.");
            return;
        }

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(physics::cast(mActor->getWorldPosition()));
        transform.setRotation(physics::cast(mActor->rotation));

        const bool isDynamic = mMass != 0.f;

        btVector3 localInertia(0.f, 0.f, 0.f);
        if (isDynamic)
            collider->getCollider()->calculateLocalInertia(mMass, localInertia);

        mMotionState = std::make_unique<btDefaultMotionState>(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mMass, mMotionState.get(), collider->getCollider(), localInertia);
        mRigidBody = std::make_unique<btRigidBody>(rbInfo);

        mRigidBody->setUserPointer(this);
        core->getPhysicsWorld()->addRigidBody(mRigidBody.get());
    }

    void RigidBody::onDrawUi()
    {
        ImGui::PushID("RigidBody");
        if (ImGui::TreeNodeEx("Rigid Body", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);

            ImGui::DragFloat("Mass", &mMass);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void RigidBody::onFixedUpdate()
    {
        btTransform transform;
        mMotionState->getWorldTransform(transform);
        mActor->position = physics::cast(transform.getOrigin());
        mActor->rotation = physics::cast(transform.getRotation());
    }

    void RigidBody::teleport()
    {
        if (!mRigidBody)
            return;

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(physics::cast(mActor->getWorldPosition()));
        transform.setRotation(physics::cast(mActor->rotation));
        mRigidBody->setWorldTransform(transform);
        mRigidBody->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
        mRigidBody->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
        mRigidBody->activate();
        mRigidBody->clearForces();
        // motionState->setWorldTransform(transform);
        // mMotionState->setWorldTransform(transform);
    }
} // engine