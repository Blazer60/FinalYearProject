/**
 * @file PhysicsCore.cpp
 * @author Ryan Purse
 * @date 06/12/2023
 */


#include "PhysicsCore.h"

#include "Actor.h"
#include "Core.h"
#include "EngineState.h"
#include "Logger.h"
#include "LoggerMacros.h"
#include "PhysicsConversions.h"

namespace engine
{
    PhysicsCore::PhysicsCore() :
        configuration(std::make_unique<btDefaultCollisionConfiguration>()),
        dispatcher(std::make_unique<btCollisionDispatcher>(configuration.get())),
        overlappingPairCache(std::make_unique<btDbvtBroadphase>()),
        solver(std::make_unique<btSequentialImpulseConstraintSolver>()),
        dynamicsWorld(std::make_unique<btDiscreteDynamicsWorld>(dispatcher.get(), overlappingPairCache.get(), solver.get(), configuration.get()))
    {
        dynamicsWorld->setGravity(btVector3(0, -9.81f, 0.f));
        dispatcher->setNearCallback([](btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo) {
            const auto object1 = static_cast<btCollisionObject*>(collisionPair.m_pProxy0->m_clientObject);
            const auto object2 = static_cast<btCollisionObject*>(collisionPair.m_pProxy1->m_clientObject);

            const auto actor1 = static_cast<Actor*>(object1->getUserPointer());
            const auto actor2 = static_cast<Actor*>(object2->getUserPointer());

            btCollisionDispatcher::defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
            btManifoldArray array;
            collisionPair.m_algorithm->getAllContactManifolds(array);
            if (array.size() > 0)
            {
                MESSAGE("Hit between: % and %", actor1->getName(), actor2->getName());
                const std::string hash = std::to_string(actor1->getId()) + std::to_string(actor2->getId());
                mCollisions[hash] = 5;
            }

            for (int i = 0; i < array.size(); ++i)
            {
                btPersistentManifold* manifold = array[i];
                const int contactCount = manifold->getNumContacts();
                for (int j = 0; j < contactCount; ++j)
                {
                    auto point = manifold->getContactPoint(j);
                    const glm::vec3 normal = physics::cast(point.m_normalWorldOnB);
                    const glm::vec3 position = physics::cast(point.getPositionWorldOnA());
                    MESSAGE("Hit normal: %, Hit Position: %", normal, position);
                }
            }
        });
    }

    PhysicsCore::~PhysicsCore()
    {
        dynamicsWorld.reset();
        solver.reset();
        overlappingPairCache.reset();
        dispatcher.reset();
        configuration.reset();
    }
}
