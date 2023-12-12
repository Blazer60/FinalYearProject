/**
 * @file PhysicsCore.cpp
 * @author Ryan Purse
 * @date 06/12/2023
 */


#include "PhysicsCore.h"

#include "Actor.h"
#include "AssimpLoader.h"
#include "Core.h"
#include "EngineState.h"
#include "Logger.h"
#include "LoggerMacros.h"
#include "PhysicsConversions.h"
#include "Mesh.h"
#include "RigidBody.h"
#include "Colliders.h"
#include "GraphicsState.h"

namespace engine
{
    void physicsNearCallback(btBroadphasePair &collisionPair, btCollisionDispatcher &dispatcher, const btDispatcherInfo &dispatcherInfo)
    {
        const auto object1 = static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject);
        const auto object2 = static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject);

        const auto componentA = static_cast<Component*>(object1->getUserPointer());
        const auto componentB = static_cast<Component*>(object2->getUserPointer());

        btCollisionDispatcher::defaultNearCallback(collisionPair, dispatcher, dispatcherInfo);
        btManifoldArray array;
        collisionPair.m_algorithm->getAllContactManifolds(array);
        if (array.size() > 0)
            physicsSystem->createHitInfo(array, componentA, componentB);
    }

    PhysicsCore::PhysicsCore() :
        configuration(std::make_unique<btDefaultCollisionConfiguration>()),
        dispatcher(std::make_unique<btCollisionDispatcher>(configuration.get())),
        overlappingPairCache(std::make_unique<btDbvtBroadphase>()),
        solver(std::make_unique<btSequentialImpulseConstraintSolver>()),
        dynamicsWorld(std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), overlappingPairCache.get(), solver.get(), configuration.get())),
        debugDrawer(std::make_unique<PhysicsDebugDrawer>()),
        mDefaultCube(load::model<PositionVertex>(file::modelPath() / "defaultObjects/DefaultCube.glb")),
        mDefaultSphere(load::model<PositionVertex>(file::modelPath() / "defaultObjects/DefaultSphere.glb"))
    {
        dynamicsWorld->setGravity(btVector3(0, -9.81f, 0.f));
        dynamicsWorld->setDebugDrawer(debugDrawer.get());
        dispatcher->setNearCallback(physicsNearCallback);
    }

    PhysicsCore::~PhysicsCore()
    {
        debugDrawer.reset();
        dynamicsWorld.reset();
        solver.reset();
        overlappingPairCache.reset();
        dispatcher.reset();
        configuration.reset();
    }

    void PhysicsCore::clearContainers()
    {
        mCollisions.clear();
        mCurrentCollisions.clear();
    }

    void PhysicsCore::renderDebugShapes() const
    {
        PROFILE_FUNC();

        if (!editor->isDebugOverlayOn())
            return;

        const int count = dynamicsWorld->getNumCollisionObjects();
        for (int i = 0; i < count; ++i)
        {
            const btCollisionObject *obj = dynamicsWorld->getCollisionObjectArray()[i];
            auto const*const rigidBody = static_cast<RigidBody*>(obj->getUserPointer());
            Actor* actor = rigidBody->getActor();
            // todo: This needs to follow the hierarchy.
            if (auto meshCollider = actor->getComponent<MeshCollider>(false); meshCollider.isValid())
            {
                const glm::mat4 modelMatrix = actor->getLocalTransform();
                graphics::renderer->drawDebugMesh(meshCollider->getDebugMesh(), modelMatrix, glm::vec3(1.f, 0.f, 1.f));
            }
            else  // Use line renderer instead.
            {
                dynamicsWorld->debugDrawObject(
                    rigidBody->mRigidBody->getWorldTransform(),
                    rigidBody->mRigidBody->getCollisionShape(),
                    physics::cast(glm::vec3(1.f, 0.f, 1.f)));
            }
        }
    }

    void PhysicsCore::resolveCollisoinCallbacks()
    {
        // Check for new hits.
        for (auto [collisionId, hitInfoExt] : mCollisions)
        {
            if (mCurrentCollisions.find(collisionId) == mCurrentCollisions.end())
            {
                // On Collision Begin.
                hitInfoExt.actorA->collisionBegin(
                    hitInfoExt.actorB, hitInfoExt.componentA,
                    hitInfoExt.componentB, HitInfo { -hitInfoExt.hitNormalWorldB, hitInfoExt.hitPositionWorldA });

                hitInfoExt.actorB->collisionBegin(
                    hitInfoExt.actorA, hitInfoExt.componentB,
                    hitInfoExt.componentA, HitInfo {  hitInfoExt.hitNormalWorldB, hitInfoExt.hitPositionWorldB });
            }

            mCurrentCollisions.insert(collisionId);  // We're using a set to avoid duplicates.
        }

        // Do continous collisions here.

        // std::erase_if() is C++20
        for (auto first = mCurrentCollisions.begin(), last = mCurrentCollisions.end(); first != last;)
        {
            if (mCollisions.find(*first) == mCollisions.end())
                first = mCurrentCollisions.erase(first);
            else
                ++first;
        }

        mCollisions.clear();
    }

    void PhysicsCore::createHitInfo(const btManifoldArray& manifoldArray, Component* componentA, Component* componentB)
    {
        const int count = manifoldArray.size();
        bool hasAnyContact = false;
        for (int i = 0; i < count; ++i)
        {
            if (auto *const manifold = manifoldArray[i]; manifold->getNumContacts() > 0)
            {
                hasAnyContact = true;
                break;
            }
        }
        if (!hasAnyContact)
            return;

        auto *const actorA = componentA->getActor();
        auto *const actorB = componentB->getActor();

        const auto hitId = std::to_string(actorA->getId()) + std::to_string(actorB->getId());

        HitInfoExt &hitInfoExt = mCollisions[hitId];
        hitInfoExt.actorA = actorA;
        hitInfoExt.actorB = actorB;
        hitInfoExt.componentA = componentA;
        hitInfoExt.componentB = componentB;
        hitInfoExt.hitPositionWorldA = glm::vec3(0.f);
        hitInfoExt.hitPositionWorldB = glm::vec3(0.f);
        hitInfoExt.hitNormalWorldB   = glm::vec3(0.f);

        for (int i = 0; i < count; ++i)
        {
            auto *const manifold = manifoldArray[i];
            const int manifoldContactCount = manifold->getNumContacts();
            glm::vec3 hitPositionWorldA = glm::vec3(0.f);
            glm::vec3 hitPositionWorldB = glm::vec3(0.f);
            glm::vec3 hitNormalWorldB   = glm::vec3(0.f);

            for (int j = 0; j < manifoldContactCount; ++j)
            {
                btManifoldPoint point = manifold->getContactPoint(j);
                hitPositionWorldA   += physics::cast(point.m_positionWorldOnA);
                hitPositionWorldB   += physics::cast(point.m_positionWorldOnB);
                hitNormalWorldB     += physics::cast(point.m_normalWorldOnB);
            }

            const float denominator = 1.f / static_cast<float>(manifoldContactCount);
            hitPositionWorldA *= denominator;
            hitPositionWorldB *= denominator;
            hitNormalWorldB   *= denominator;

            hitInfoExt.hitPositionWorldA += hitPositionWorldA;
            hitInfoExt.hitPositionWorldB += hitPositionWorldB;
            hitInfoExt.hitNormalWorldB   += hitNormalWorldB;
        }

        const float oneOverCount = 1.f / static_cast<float>(count);
        hitInfoExt.hitPositionWorldA *= oneOverCount;
        hitInfoExt.hitPositionWorldB *= oneOverCount;
        hitInfoExt.hitNormalWorldB   *= oneOverCount;
    }
}
