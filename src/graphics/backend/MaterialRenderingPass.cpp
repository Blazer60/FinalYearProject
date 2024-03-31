/**
 * @file MaterialRenderingPass.cpp
 * @author Ryan Purse
 * @date 09/03/2024
 */


#include "MaterialRenderingPass.h"

#include "GraphicsFunctions.h"
#include "WindowHelpers.h"

namespace graphics
{
    void MaterialRenderingPass::execute(
            const glm::ivec2 &size, Context &context,
            const std::vector<GeometryObject>&multiGeometryQueue, const std::vector<MaterialData>&multiMaterialQueue,
            const std::vector<GeometryObject>&singleGeometryQueue, const std::vector<MaterialData>&singleMaterialQueue)
    {
        PROFILE_FUNC();
        if (multiGeometryQueue.size() != multiMaterialQueue.size() || singleGeometryQueue.size() != singleMaterialQueue.size())
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
        mMaskShaderStorage.bindToSlot(3);

        executeSingleMaterial(context, singleGeometryQueue, singleMaterialQueue);
        executeMultiMaterial(context, multiGeometryQueue, multiMaterialQueue);

        mFramebuffer.detach(0);
        mFramebuffer.detach(1);
        mFramebuffer.detach(2);
        mFramebuffer.detachDepthBuffer();

        popDebugGroup();
    }

    void MaterialRenderingPass::executeMultiMaterial(
        const Context& context,
        const std::vector<GeometryObject>& geometryQueue, const std::vector<MaterialData>& materials)
    {
        mMultiMaterialShader.bind();
        mMultiMaterialShader.block("CameraBlock", context.camera.getBindPoint());

        for (int i = 0; i < geometryQueue.size(); ++i)
        {
            const GeometryObject &geometry = geometryQueue[i];
            const MaterialData &material = materials[i];

            // todo: These two should really be a UBO...
            mMultiMaterialShader.set("u_mvp_matrix", context.cameraViewProjectionMatrix * geometry.matrix);
            mMultiMaterialShader.set("u_model_matrix", geometry.matrix);
            mMultiMaterialShader.set("textures", material.textureArrayId, 0);

            mMaterialShaderStorage.resize(sizeof(LayerData) * material.layers.size());
            mMaterialShaderStorage.write(material.layers.data(), sizeof(LayerData) * material.layers.size());

            mTextureDataShaderStorage.resize(sizeof(TextureData) * material.textureArrayData.size());
            mTextureDataShaderStorage.write(material.textureArrayData.data(), sizeof(TextureData) * material.textureArrayData.size());

            mMaskShaderStorage.resize(sizeof(MaskData) * material.masks.size());
            mMaskShaderStorage.write(material.masks.data(), sizeof(MaskData) * material.masks.size());

            glBindVertexArray(geometry.vao);
            glDrawElements(GL_TRIANGLES, geometry.indicesCount, GL_UNSIGNED_INT, nullptr);
        }
    }

    void MaterialRenderingPass::executeSingleMaterial(
        const Context& context,
        const std::vector<GeometryObject>& geometryQueue, const std::vector<MaterialData>& materials)
    {
        mSingleMaterialShader.bind();
        mSingleMaterialShader.block("CameraBlock", context.camera.getBindPoint());

        for (int i = 0; i < geometryQueue.size(); ++i)
        {
            const GeometryObject &geometry = geometryQueue[i];
            const MaterialData &material = materials[i];

            if (material.layers.empty())
                CRASH("No layers to read from results in undefined behavour.");

            // todo: These two should really be a UBO...
            mSingleMaterialShader.set("u_mvp_matrix", context.cameraViewProjectionMatrix * geometry.matrix);
            mSingleMaterialShader.set("u_model_matrix", geometry.matrix);
            mSingleMaterialShader.set("textures", material.textureArrayId, 0);

            // Only read in the first layer as that's the only one we can safely use.
            mMaterialShaderStorage.resize(sizeof(LayerData));
            mMaterialShaderStorage.write(material.layers.data(), sizeof(LayerData));

            mTextureDataShaderStorage.resize(sizeof(TextureData) * material.textureArrayData.size());
            mTextureDataShaderStorage.write(material.textureArrayData.data(), sizeof(TextureData) * material.textureArrayData.size());

            glBindVertexArray(geometry.vao);
            glDrawElements(GL_TRIANGLES, geometry.indicesCount, GL_UNSIGNED_INT, nullptr);
        }
    }
}
