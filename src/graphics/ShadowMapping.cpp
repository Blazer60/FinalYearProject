/**
 * @file ShadowMapping.cpp
 * @author Ryan Purse
 * @date 03/07/2023
 */


#include "ShadowMapping.h"
#include "RendererData.h"
#include "WindowHelpers.h"
#include "Pch.h"

namespace renderer
{
    namespace shadow
    {
        float zMultiplier { 2.f };
        std::vector<float> cascadeMultipliers { 0.125f, 0.25f };
    }
    
    using namespace shadow;
    void shadowMapping(const CameraSettings &cameraSettings, const std::vector<float> &cascadeDepths)
    {
        const auto resize = [](const glm::vec4 &v) { return v / v.w; };
        
        shadowFramebuffer->bind();
        shadowShader->bind();
        
        for (DirectionalLight &directionalLight : directionalLightQueue)
        {
            const glm::ivec2 &shadowMapSize = directionalLight.shadowMap->getSize();
            glViewport(0, 0, shadowMapSize.x, shadowMapSize.y);
            
            std::vector<float> depths { cameraSettings.nearClipDistance };
            for (const float &depth : cascadeDepths)
                depths.emplace_back(depth);
            depths.emplace_back(cameraSettings.farClipDistance);
            
            for (int j = 0; j < directionalLight.shadowMap->getLayerCount(); ++j)
            {
                shadowFramebuffer->attachDepthBuffer(*directionalLight.shadowMap, j);
                shadowFramebuffer->clear(glm::vec4(glm::vec3(0.f), 1.f));
                
                const float aspectRatio = window::aspectRatio();
                const glm::mat4 projectionMatrix = glm::perspective(cameraSettings.fovY, aspectRatio, depths[j], depths[j + 1]);
                
                // We only want the shadow map to encompass the camera's frustum.
                const glm::mat4 inverseVpMatrix = glm::inverse(projectionMatrix * cameraSettings.viewMatrix);
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
                    minLightSpacePoint.z *= zMultiplier;
                else
                    minLightSpacePoint.z /= zMultiplier;
                if (maxLightSpacePoint.z < 0)
                    maxLightSpacePoint.z /= zMultiplier;
                else
                    maxLightSpacePoint.z *= zMultiplier;
                
                const glm::mat4 lightProjectionMatrix = glm::ortho(
                    minLightSpacePoint.x, maxLightSpacePoint.x, minLightSpacePoint.y, maxLightSpacePoint.y,
                    minLightSpacePoint.z, maxLightSpacePoint.z
                );
                
                directionalLight.vpMatrices.emplace_back(lightProjectionMatrix * lightViewMatrix);
                
                for (const auto &rqo : renderQueue)
                {
                    const glm::mat4 &modelMatrix = rqo.matrix;
                    const glm::mat4 mvp = lightProjectionMatrix * lightViewMatrix * modelMatrix;
                    shadowShader->set("u_mvp_matrix", mvp);
                    glBindVertexArray(rqo.vao);
                    glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
                }
                
                shadowFramebuffer->detachDepthBuffer();
            }
        }
        
        // Reset the viewport back to the normal size once we've finished rendering all the shadows.
        glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
    }
}
