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

    void RigidBody::setupRigidBody(btCollisionShape* collisionShape)
    {
        if (mRigidBody || !core->isInPlayMode())
            return;  // We have already done setup.

        if (collisionShape == nullptr)
        {
            auto colliderComponent = mActor->getComponent<Collider>(false);
            if (!colliderComponent.isValid())
            {
                WARN("A Rigid Body is attached to an actor but no collider is present.");
                return;
            }
            collisionShape = colliderComponent->getCollider();
        }

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(physics::cast(mActor->getWorldPosition()));
        transform.setRotation(physics::cast(mActor->getWorldRotation()));

        const bool isDynamic = mMass != 0.f;

        btVector3 localInertia(0.f, 0.f, 0.f);
        if (isDynamic)
            collisionShape->calculateLocalInertia(mMass, localInertia);

        mMotionState = std::make_unique<btDefaultMotionState>(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mMass, mMotionState.get(), collisionShape, localInertia);
        rbInfo.m_friction = mFriction;
        mRigidBody = std::make_unique<btRigidBody>(rbInfo);
        setAngularFactor(mAngularFactor);

        addToPhysicsWorld();
    }

    void RigidBody::onAwake()
    {
        setupRigidBody();
    }

    void RigidBody::addToPhysicsWorld()
    {
        mRigidBody->setUserPointer(this);
        core->getPhysicsWorld()->addRigidBody(mRigidBody.get(), mGroupMask, mCollisionMask);
    }

    void RigidBody::onBegin()
    {
    }

    void RigidBody::setMass(const float mass)
    {
        mMass = mass;
        if (mRigidBody)
            mRigidBody->setMassProps(mass, mRigidBody->getLocalInertia());
    }

    void RigidBody::onDrawUi()
    {
        ImGui::PushID("RigidBody");
        if (ImGui::TreeNodeEx("Rigid Body", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Component"))
                mActor->removeComponent(this);

            if (ImGui::DragFloat("Mass", &mMass))
                setMass(mMass);
            if (ImGui::DragFloat("Friction", &mFriction))
                setFriction(mFriction);
            if (ImGui::DragFloat3("Angular Factor", glm::value_ptr(mAngularFactor)))
                setAngularFactor(mAngularFactor);

            auto drawRow = [this](const std::string &name, const int flag) {
                ImGui::PushID(name.c_str());
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text(name.c_str());
                ImGui::TableNextColumn();
                ImGui::CheckboxFlags("##GroupMask", &mGroupMask, flag);
                ImGui::TableNextColumn();
                ImGui::CheckboxFlags("##ColliderMask", &mCollisionMask, flag);
                ImGui::PopID();
            };

            ImGui::Checkbox("Is Trigger?", &mIsTrigger);
            if (ImGui::BeginTable("Collision Mask Matrix", 3))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Group Mask");
                ImGui::TableSetupColumn("Collision Mask");
                ImGui::TableHeadersRow();
                drawRow("Default Filter",    btBroadphaseProxy::DefaultFilter);
                drawRow("Static Filer",      btBroadphaseProxy::StaticFilter);
                drawRow("Kinematic Filter",  btBroadphaseProxy::KinematicFilter);
                drawRow("Debris Filter",     btBroadphaseProxy::DebrisFilter);
                drawRow("Sensor Trigger",    btBroadphaseProxy::SensorTrigger);
                drawRow("Character Filter",  btBroadphaseProxy::CharacterFilter);

                ImGui::EndTable();
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void RigidBody::alignWithActorTransform() const
    {
        if (!mRigidBody)
            return;

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(physics::cast(mActor->getWorldPosition()));
        transform.setRotation(physics::cast(mActor->getWorldRotation()));
        mRigidBody->setWorldTransform(transform);
        mRigidBody->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
        mRigidBody->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
        mRigidBody->activate();
        mRigidBody->clearForces();
    }

    void RigidBody::setGroupMask(const int mask)
    {
        mGroupMask = mask;
    }

    void RigidBody::setCollisionMask(const int mask)
    {
        mCollisionMask = mask;
    }

    void RigidBody::setIsTrigger(const bool isTrigger)
    {
        mIsTrigger = isTrigger;
    }

    bool RigidBody::isTrigger() const
    {
        return mIsTrigger;
    }

    void RigidBody::addImpulse(const glm::vec3 &impulse) const
    {
        mRigidBody->applyCentralImpulse(physics::cast(impulse));
    }

    void RigidBody::active() const
    {
        mRigidBody->activate();
    }

    void RigidBody::setAngularFactor(const glm::vec3& angularFactor)
    {
        mAngularFactor = angularFactor;
        if (mRigidBody)
            mRigidBody->setAngularFactor(physics::cast(angularFactor));
    }

    void RigidBody::setFriction(const float friction)
    {
        mFriction = friction;
        if (mRigidBody)
            mRigidBody->setFriction(friction);
    }
} // engine