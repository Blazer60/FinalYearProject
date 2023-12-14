/**
 * @file RigidBody.h
 * @author Ryan Purse
 * @date 07/12/2023
 */


#pragma once

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>

#include "Component.h"
#include "Pch.h"

namespace engine
{
    class Collider;

    class RigidBody
        : public Component
    {
        friend class PhysicsCore;
    public:
        RigidBody() = default;
        explicit RigidBody(float mass);
        ~RigidBody() override;

        void onBegin() override;
        void onDrawUi() override;
        void onFixedUpdate() override;
        void alignWithActorTransform();
        void setGroupMask(int mask);
        void setCollisionMask(int mask);
        void setIsTrigger(bool isTrigger);
        bool isTrigger() const;

        void addImpulse(const glm::vec3 &impulse) const;
        void active() const;
        void setAngularFactor(const glm::vec3 &angularFactor) const;
        void setFriction(float friction) const;

    protected:
        void addToPhysicsWorld();

        float mMass { 10.f };
        std::unique_ptr<btRigidBody> mRigidBody;
        std::unique_ptr<btDefaultMotionState> mMotionState;
        Ref<Collider> mCollider;
        int mGroupMask { btBroadphaseProxy::DefaultFilter };
        int mCollisionMask { btBroadphaseProxy::AllFilter };
        bool mIsTrigger { false };

        ENGINE_SERIALIZABLE_COMPONENT(RigidBody);
    };
}
