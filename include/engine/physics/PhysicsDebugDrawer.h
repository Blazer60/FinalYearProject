/**
 * @file PhysicsDebugDrawer.h
 * @author Ryan Purse
 * @date 12/12/2023
 */


#pragma once

#include <LinearMath/btIDebugDraw.h>

#include "Pch.h"

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 12/12/2023
     */
    class PhysicsDebugDrawer
        : public btIDebugDraw
    {
    public:
        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

        void drawContactPoint(
            const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
            const btVector3& color) override;

        void reportErrorWarning(const char* warningString) override;

        void draw3dText(const btVector3& location, const char* textString) override;

        void setDebugMode(int debugMode) override;

        int getDebugMode() const override;
    };

} // engine
