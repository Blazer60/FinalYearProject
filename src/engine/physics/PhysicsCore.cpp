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
        mDefaultCube(load::model<PositionVertex>(file::modelPath() / "defaultObjects/DefaultCube.glb")),
        mDefaultSphere(load::model<PositionVertex>(file::modelPath() / "defaultObjects/DefaultSphere.glb"))
    {
        dynamicsWorld->setGravity(btVector3(0, -9.81f, 0.f));
        dispatcher->setNearCallback(physicsNearCallback);
    }

    PhysicsCore::~PhysicsCore()
    {
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

        const int count = dynamicsWorld->getNumCollisionObjects();
        for (int i = 0; i < count; ++i)
        {
            const btCollisionObject *obj = dynamicsWorld->getCollisionObjectArray()[i];
            auto const*const rigidBody = static_cast<RigidBody*>(obj->getUserPointer());
            Actor* actor = rigidBody->getActor();
            // todo: This needs to follow the hierarchy.
            const glm::vec3 position = actor->getWorldPosition();
            const glm::quat rotation = actor->rotation;
            const glm::mat4 actorTransform = glm::translate(glm::mat4(1.f), position) *  glm::mat4_cast(rotation);
            if (auto boxCollider = actor->getComponent<BoxCollider>(false); boxCollider.isValid())
            {
                const glm::mat4 scale = glm::scale(glm::mat4(1.f), boxCollider->getHalfExtent());
                const glm::mat4 modelMatrix = actorTransform * scale;
                graphics::renderer->drawDebugMesh(mDefaultCube, modelMatrix, glm::vec3(1.f, 0.f, 1.f));
            }
            else if (auto sphereCollider = actor->getComponent<SphereCollider>(false); sphereCollider.isValid())
            {
                const glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(sphereCollider->getRadius()));
                const glm::mat4 modelMatrix = actorTransform * scale;
                graphics::renderer->drawDebugMesh(mDefaultSphere, modelMatrix, glm::vec3(1.f, 0.f, 1.f));
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

        const int count = manifoldArray.size();
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
