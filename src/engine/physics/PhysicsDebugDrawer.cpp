/**
 * @file PhysicsDebugDrawer.cpp
 * @author Ryan Purse
 * @date 12/12/2023
 */


#include "PhysicsDebugDrawer.h"

#include "GraphicsState.h"
#include "PhysicsConversions.h"

namespace engine
{
    void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        graphics::renderer->drawDebugLine(physics::cast(from), physics::cast(to), physics::cast(color));
    }

    void PhysicsDebugDrawer::drawContactPoint(
        const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
    {

    }

    void PhysicsDebugDrawer::reportErrorWarning(const char* warningString)
    {

    }

    void PhysicsDebugDrawer::draw3dText(const btVector3& location, const char* textString)
    {

    }

    void PhysicsDebugDrawer::setDebugMode(int debugMode)
    {

    }

    int PhysicsDebugDrawer::getDebugMode() const
    {
        return DBG_DrawWireframe | DBG_DrawAabb | DBG_DrawFrames;
    }
}

