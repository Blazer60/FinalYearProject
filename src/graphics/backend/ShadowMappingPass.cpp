/**
 * @file ShadowMapping.cpp
 * @author Ryan Purse
 * @date 09/03/2024
 */


#include "ShadowMappingPass.h"

#include "Cubemap.h"
#include "GraphicsFunctions.h"
#include "WindowHelpers.h"

namespace graphics
{
    void ShadowMappingPass::execute(
        const std::vector<GeometryObject> &multiGeometryQueue,
        const std::vector<GeometryObject> &singleGeometryQueue,
        std::vector<PointLight> &pointLightQueue)
    {
        if (pointLightQueue.empty())
            return;

        PROFILE_FUNC();
        pushDebugGroup("Point Light Shadow Mapping");
        mFramebuffer.bind();
        mPointLightShadowShader.bind();

        for (auto &pointLight : pointLightQueue)
        {
            PROFILE_SCOPE_BEGIN(pointLightTimer, "Point Light Shadow Pass");
            pushDebugGroup("Point Light Pass");
            const glm::ivec2 size = pointLight.shadowMap->getSize();
            glViewport(0, 0, size.x, size.y);

            mPointLightShadowShader.set("u_light_pos", pointLight.position);
            mPointLightShadowShader.set("u_z_far", pointLight.radius);

            for (int viewIndex = 0; viewIndex < 6; ++viewIndex)
            {
                pushDebugGroup("Rendering Face");
                Cubemap &shadowMap = *pointLight.shadowMap;
                mFramebuffer.attachDepthBuffer(shadowMap, viewIndex, 0);
                mFramebuffer.clearDepthBuffer();

                for (const auto &geometry : multiGeometryQueue)
                {
                    const glm::mat4 &modelMatrix = geometry.matrix;
                    const glm::mat4 mvp = pointLight.vpMatrices[viewIndex] * modelMatrix;
                    mPointLightShadowShader.set("u_model_matrix", modelMatrix);
                    mPointLightShadowShader.set("u_mvp_matrix", mvp);

                    glBindVertexArray(geometry.vao);
                    glDrawElements(GL_TRIANGLES, geometry.indicesCount, GL_UNSIGNED_INT, nullptr);
                }

                for (const auto &geometry : singleGeometryQueue)
                {
                    const glm::mat4 &modelMatrix = geometry.matrix;
                    const glm::mat4 mvp = pointLight.vpMatrices[viewIndex] * modelMatrix;
                    mPointLightShadowShader.set("u_model_matrix", modelMatrix);
                    mPointLightShadowShader.set("u_mvp_matrix", mvp);

                    glBindVertexArray(geometry.vao);
                    glDrawElements(GL_TRIANGLES, geometry.indicesCount, GL_UNSIGNED_INT, nullptr);
                }

                mFramebuffer.detachDepthBuffer();
                popDebugGroup();
            }

            PROFILE_SCOPE_END(pointLightTimer);
            popDebugGroup();
        }

        popDebugGroup();
    }

    void ShadowMappingPass::execute(
        const std::vector<GeometryObject> &multiGeometryQueue,
        const std::vector<GeometryObject> &singleGeometryQueue,
        const std::vector<Spotlight> &spotlightQueue)
    {
        if (spotlightQueue.empty())
            return;

        PROFILE_FUNC();
        pushDebugGroup("Spotlight Shadow Mapping");

        mSpotlightShadowShader.bind();
        mFramebuffer.bind();

        for (const Spotlight &spotlight : spotlightQueue)
        {
            const glm::ivec2 size = spotlight.shadowMap->getSize();
            glViewport(0, 0, size.x, size.y);

            mFramebuffer.attachDepthBuffer(spotlight.shadowMap.get());
            mFramebuffer.clearDepthBuffer();

            const glm::mat4 &vpMatrix = spotlight.vpMatrix;
            mSpotlightShadowShader.set("u_light_pos", spotlight.position);
            mSpotlightShadowShader.set("u_z_far", spotlight.radius);

            for (const auto &rqo : multiGeometryQueue)
            {
                const glm::mat4 mvpMatrix = vpMatrix * rqo.matrix;
                mSpotlightShadowShader.set("u_mvp_matrix", mvpMatrix);
                mSpotlightShadowShader.set("u_model_matrix", rqo.matrix);

                glBindVertexArray(rqo.vao);
                glDrawElements(GL_TRIANGLES, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
            }

            for (const auto &rqo : singleGeometryQueue)
            {
                const glm::mat4 mvpMatrix = vpMatrix * rqo.matrix;
                mSpotlightShadowShader.set("u_mvp_matrix", mvpMatrix);
                mSpotlightShadowShader.set("u_model_matrix", rqo.matrix);

                glBindVertexArray(rqo.vao);
                glDrawElements(GL_TRIANGLES, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
            }

            mFramebuffer.detachDepthBuffer();
        }

        popDebugGroup();
    }

    void ShadowMappingPass::execute(
        const CameraSettings &camera,
        const std::vector<GeometryObject> &multiGeometryQueue,
        const std::vector<GeometryObject> &singleGeometryQueue,
        std::vector<DirectionalLight> &directionalLightQueue)
    {
        if (directionalLightQueue.empty())
            return;

        PROFILE_FUNC();
        pushDebugGroup("Directional Light Shadow Mapping");
        const auto resize = [](const glm::vec4 &vec) { return vec / vec.w; };

        mFramebuffer.bind();
        mDirectionalLightShadowShader.bind();

        for (DirectionalLight &directionalLight : directionalLightQueue)
        {
            directionalLight.cascadeDepths.clear();
            directionalLight.cascadeDepths.reserve(directionalLight.shadowCascadeMultipliers.size());
            for (const auto &multiplier : directionalLight.shadowCascadeMultipliers)
                directionalLight.cascadeDepths.emplace_back(camera.farClipDistance * multiplier);

            graphics::pushDebugGroup("Directional Light");

            const glm::ivec2 &shadowMapSize = directionalLight.shadowMap->getSize();
            glViewport(0, 0, shadowMapSize.x, shadowMapSize.y);

            std::vector depths { camera.nearClipDistance };
            for (const float &depth : directionalLight.cascadeDepths)
                depths.emplace_back(depth);
            depths.emplace_back(camera.farClipDistance);

            for (int j = 0; j < directionalLight.shadowMap->getLayerCount(); ++j)
            {
                pushDebugGroup("Cascade Pass");

                mFramebuffer.attachDepthBuffer(*directionalLight.shadowMap, j);
                mFramebuffer.clearDepthBuffer();

                const float aspectRatio = window::aspectRatio();
                const glm::mat4 projectionMatrix = glm::perspective(camera.fovY, aspectRatio, depths[j], depths[j + 1]);

                // We only want the shadow map to encompass the camera's frustum.
                const glm::mat4 inverseVpMatrix = glm::inverse(projectionMatrix * camera.viewMatrix);
                const std::vector<glm::vec4> worldPoints = {
                    resize(inverseVpMatrix * glm::vec4(1.f, 1.f, -1.f, 1.f)),
                    resize(inverseVpMatrix * glm::vec4(1.f, -1.f, -1.f, 1.f)),
                    resize(inverseVpMatrix * glm::vec4(-1.f, 1.f, -1.f, 1.f)),
                    resize(inverseVpMatrix * glm::vec4(-1.f, -1.f, -1.f, 1.f)),

                    resize(inverseVpMatrix * glm::vec4(1.f, 1.f, 1.f, 1.f)),
                    resize(inverseVpMatrix * glm::vec4(1.f, -1.f, 1.f, 1.f)),
                    resize(inverseVpMatrix * glm::vec4(-1.f, 1.f, 1.f, 1.f)),
                    resize(inverseVpMatrix * glm::vec4(-1.f, -1.f, 1.f, 1.f)),
                };

                glm::vec3 minWorldBound = worldPoints[0];
                glm::vec3 maxWorldBound = worldPoints[0];
                for (int i = 1; i < worldPoints.size(); ++i)
                {
                    minWorldBound = glm::min(minWorldBound, glm::vec3(worldPoints[i]));
                    maxWorldBound = glm::max(maxWorldBound, glm::vec3(worldPoints[i]));
                }
                const glm::vec3 centerPoint = minWorldBound + 0.5f * (maxWorldBound - minWorldBound);

                const glm::mat4 lightViewMatrix = glm::lookAt(
                    centerPoint + directionalLight.direction, centerPoint, glm::vec3(0.f, 1.f, 0.f));
                glm::vec3 minLightSpacePoint = lightViewMatrix * worldPoints[0];
                glm::vec3 maxLightSpacePoint = lightViewMatrix * worldPoints[0];
                for (int i = 1; i < worldPoints.size(); ++i)
                {
                    const glm::vec3 lightPoint = lightViewMatrix * worldPoints[i];
                    minLightSpacePoint = glm::min(minLightSpacePoint, lightPoint);
                    maxLightSpacePoint = glm::max(maxLightSpacePoint, lightPoint);
                }

                if (minLightSpacePoint.z < 0)
                    minLightSpacePoint.z *= directionalLight.shadowZMultiplier;
                else
                    minLightSpacePoint.z /= directionalLight.shadowZMultiplier;
                if (maxLightSpacePoint.z < 0)
                    maxLightSpacePoint.z /= directionalLight.shadowZMultiplier;
                else
                    maxLightSpacePoint.z *= directionalLight.shadowZMultiplier;

                const glm::mat4 lightProjectionMatrix = glm::ortho(
                    minLightSpacePoint.x, maxLightSpacePoint.x, minLightSpacePoint.y, maxLightSpacePoint.y,
                    minLightSpacePoint.z, maxLightSpacePoint.z
                );

                directionalLight.vpMatrices.emplace_back(lightProjectionMatrix * lightViewMatrix);

                for (const auto &rqo : multiGeometryQueue)
                {
                    const glm::mat4 &modelMatrix = rqo.matrix;
                    const glm::mat4 mvp = lightProjectionMatrix * lightViewMatrix * modelMatrix;
                    mDirectionalLightShadowShader.set("u_mvp_matrix", mvp);

                    glBindVertexArray(rqo.vao);
                    glDrawElements(GL_TRIANGLES, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
                }

                for (const auto &rqo : singleGeometryQueue)
                {
                    const glm::mat4 &modelMatrix = rqo.matrix;
                    const glm::mat4 mvp = lightProjectionMatrix * lightViewMatrix * modelMatrix;
                    mDirectionalLightShadowShader.set("u_mvp_matrix", mvp);

                    glBindVertexArray(rqo.vao);
                    glDrawElements(GL_TRIANGLES, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
                }

                mFramebuffer.detachDepthBuffer();
                popDebugGroup();
            }

            popDebugGroup();
        }

        popDebugGroup();
    }
}
