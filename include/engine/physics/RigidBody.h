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
#include "Serializer.h"

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

        void setupRigidBody(btCollisionShape *collisionShape=nullptr);
        void onAwake() override;
        void onBegin() override;

        void setMass(float mass);

        void onDrawUi() override;

        void alignWithActorTransform() const;
        void setGroupMask(int mask);
        void setCollisionMask(int mask);
        void setIsTrigger(bool isTrigger);
        bool isTrigger() const;

        void addImpulse(const glm::vec3 &impulse) const;
        void active() const;
        void setAngularFactor(const glm::vec3 &angularFactor);
        void setFriction(float friction);

    protected:
        void addToPhysicsWorld();

        float mMass { 10.f };
        float mFriction { 1.f };
        glm::vec3 mAngularFactor { 1.f };
        std::unique_ptr<btRigidBody> mRigidBody;

        // Warning: Use the btRigidBody before this. Looks like bullet doesn't keep them synced.
        std::unique_ptr<btDefaultMotionState> mMotionState;
        int mGroupMask { btBroadphaseProxy::DefaultFilter };
        int mCollisionMask { btBroadphaseProxy::AllFilter };
        bool mIsTrigger { false };

        ENGINE_SERIALIZABLE_COMPONENT(RigidBody);
    };
}
