/**
 * @file PhysicsCore.cpp
 * @author Ryan Purse
 * @date 06/12/2023
 */


#include "PhysicsCore.h"

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
