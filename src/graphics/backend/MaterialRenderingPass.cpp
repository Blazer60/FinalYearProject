/**
 * @file MaterialRendering.cpp
 * @author Ryan Purse
 * @date 09/03/2024
 */


#include "MaterialRenderingPass.h"

#include "GraphicsFunctions.h"
#include "WindowHelpers.h"

namespace graphics
{
    void MaterialRenderingPass::execute(const glm::ivec2 &size, Context &context, const std::vector<RenderQueueObject> &renderQueue)
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

        setViewport(size);

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

    void MaterialRenderingPass::execute(
        const glm::ivec2& size, Context& context,
        const std::vector<GeometryObject>& geometryQueue, const std::vector<MaterialData>& materials)
    {
        PROFILE_FUNC();
        if (geometryQueue.size() != materials.size())
            CRASH("Geometry material size missmatch!");

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

        setViewport(size);

        // Bind Uniform Buffer Objects. Can this be done at a global level?
        context.camera.bindToSlot(0);
        mMaterialShaderStorage.bindToSlot(1);
        mTextureDataShaderStorage.bindToSlot(2);

        mMaterialShader.bind();
        mMaterialShader.block("CameraBlock", context.camera.getBindPoint());

        for (int i = 0; i < geometryQueue.size(); ++i)
        {
            const GeometryObject &geometry = geometryQueue[i];
            const MaterialData &material = materials[i];

            // todo: These two should really be a UBO...
            mMaterialShader.set("u_mvp_matrix", context.cameraViewProjectionMatrix * geometry.matrix);
            mMaterialShader.set("u_model_matrix", geometry.matrix);
            mMaterialShader.set("textures", material.textureArrayId, 0);

            mMaterialShaderStorage.resize(sizeof(LayerData) * material.layers.size());
            mMaterialShaderStorage.write(material.layers.data(), sizeof(LayerData) * material.layers.size());

            mTextureDataShaderStorage.resize(sizeof(TextureData) * material.textureArrayData.size());
            mTextureDataShaderStorage.write(material.textureArrayData.data(), sizeof(TextureData) * material.textureArrayData.size());

            glBindVertexArray(geometry.vao);
            glDrawElements(GL_TRIANGLES, geometry.indicesCount, GL_UNSIGNED_INT, nullptr);
        }

        mFramebuffer.detach(0);
        mFramebuffer.detach(1);
        mFramebuffer.detach(2);
        mFramebuffer.detachDepthBuffer();

        popDebugGroup();
    }
}
