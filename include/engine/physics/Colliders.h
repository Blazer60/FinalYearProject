/**
 * @file Colliders.h
 * @author Ryan Purse
 * @date 06/12/2023
 */


#pragma once

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>

#include "Component.h"
#include "Pch.h"

namespace engine
{
    class Collider
        : public Component
    {
    public:
        ~Collider() override = default;

        virtual btCollisionShape *getCollider() = 0;
    };

    class BoxCollider
        : public Collider
    {
    public:
        BoxCollider();
        explicit BoxCollider(const glm::vec3 &extent);
        ~BoxCollider() override = default;

        void onDrawUi() override;
        btCollisionShape *getCollider() override;
        [[nodiscard]] glm::vec3 getHalfExtent() const;

    protected:
        glm::vec3 mHalfExtent { 0.5f };
        btBoxShape mBoxCollider;

        ENGINE_SERIALIZABLE_COMPONENT(BoxCollider);
    };

    class SphereCollider
        : public Collider
    {
    public:
        SphereCollider();
        explicit SphereCollider(float radius);
        ~SphereCollider() override = default;

        void onDrawUi() override;
        btCollisionShape *getCollider() override;
        [[nodiscard]] float getRadius() const;

    protected:
        float mRadius { 1.f };
        btSphereShape mSphereShape;

        ENGINE_SERIALIZABLE_COMPONENT(SphereCollider);
    };
}
