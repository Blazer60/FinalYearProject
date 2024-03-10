/**
 * @file MaterialRendering.h
 * @author Ryan Purse
 * @date 09/03/2024
 */


#pragma once

#include "CameraSettings.h"
#include "Context.h"
#include "GraphicsDefinitions.h"
#include "Pch.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 09/03/2024
     */
    class MaterialRenderingPass
    {
    public:
        void execute(const glm::ivec2 &size, Context &context, const std::vector<RenderQueueObject> &renderQueue);
    protected:
        FramebufferObject mFramebuffer = FramebufferObject(GL_ONE, GL_ZERO, GL_LESS);
    };
} // graphics
