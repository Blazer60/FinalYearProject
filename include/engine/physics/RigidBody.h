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
        void teleport();

    protected:
        float mMass { 10.f };
        std::unique_ptr<btRigidBody> mRigidBody;
        std::unique_ptr<btDefaultMotionState> mMotionState;
        Ref<Collider> mCollider;

        ENGINE_SERIALIZABLE_COMPONENT(RigidBody);
    };
}
