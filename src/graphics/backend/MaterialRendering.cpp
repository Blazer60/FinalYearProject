/**
 * @file MaterialRendering.cpp
 * @author Ryan Purse
 * @date 09/03/2024
 */


#include "MaterialRendering.h"

#include "GraphicsFunctions.h"
#include "WindowHelpers.h"

namespace graphics
{
    void MaterialRendering::execute(const glm::ivec2 &size, Context &context, const std::vector<RenderQueueObject> &renderQueue)
    {
        PROFILE_FUNC();
        pushDebugGroup("Material Rendering");

        context.gbuffer.resize(size);
        context.depthBuffer.resize(size);

        context.gbuffer.clear();
        context.depthBuffer.clear();

        mFramebuffer.attach(&context.gbuffer, 0, 0);
        mFramebuffer.attach(&context.gbuffer, 1, 1);
        mFramebuffer.attach(&context.gbuffer, 2, 2);
        mFramebuffer.attachDepthBuffer(&context.depthBuffer);
        mFramebuffer.bind();

        // Todo: make this a helper function.
        glViewport(0, 0, size.x, size.y);

        // Bind Uniform Buffer Objects. Can this be done at a global level?
        context.camera.bindToSlot(0);

        for (const auto & [vao, indicesCount, refShader, drawMode, matrix, onDraw] : renderQueue)
        {
            if (refShader.expired())
                continue;

            const auto shader = refShader.lock();
            shader->bind();
            shader->set("u_mvp_matrix", context.cameraViewProjectionMatrix * matrix);
            shader->set("u_model_matrix", matrix);
            shader->block("CameraBlock", context.camera.getBindPoint());

            onDraw();
            glBindVertexArray(vao);
            glDrawElements(drawMode, indicesCount, GL_UNSIGNED_INT, nullptr);
        }

        mFramebuffer.detach(0);
        mFramebuffer.detach(1);
        mFramebuffer.detach(2);
        mFramebuffer.detachDepthBuffer();

        popDebugGroup();
    }
}
