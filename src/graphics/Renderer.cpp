/**
 * @file Renderer.cpp
 * @author Ryan Purse
 * @date 14/07/2023
 */


#include "Renderer.h"
#include "WindowHelpers.h"
#include "Primitives.h"
#include "GraphicsFunctions.h"
#include "Buffers.h"
#include "Shader.h"
#include "ProfileTimer.h"
#include "FileLoader.h"

Renderer::Renderer() :
    isOk(true),
    mCurrentRenderBufferSize(window::bufferSize()),
    mFullscreenTriangle(primitives::fullscreenTriangle()),
    mUnitSphere(primitives::invertedSphere())
{
    // Blending texture data / enabling lerping.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    mDirectionalLightShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "lighting/DirectionalLight.frag");
    mPointLightShader = std::make_unique<Shader>(file::shaderPath() / "lighting/PointLight.vert", file::shaderPath() / "lighting/PointLight.frag");
    mSpotlightShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "lighting/SpotLight.frag");
    mIblShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "lighting/IBL.frag");
    mDeferredLightShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "lighting/CombineOutput.frag");
    mDirectionalLightShadowShader = std::make_unique<Shader>(file::shaderPath() / "shadow/Shadow.vert", file::shaderPath() / "shadow/Shadow.frag");
    mPointLightShadowShader = std::make_unique<Shader>(file::shaderPath() / "shadow/PointShadow.vert", file::shaderPath() / "shadow/PointShadow.frag");
    mSpotlightShadowShader = std::make_unique<Shader>(file::shaderPath() / "shadow/PointShadow.vert", file::shaderPath() / "shadow/PointShadow.frag");
    mHdrToCubemapShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "cubemap/ToCubemap.frag");
    mCubemapToIrradianceShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "cubemap/IrradianceMap.frag");
    mPreFilterShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() /  "cubemap/PreFilter.frag");
    mIntegrateBrdfShader = std::make_unique<Shader>(file::shaderPath() /  "FullscreenTriangle.vert", file::shaderPath() / "brdf/IntegrateBrdf.frag");
    mScreenSpaceReflectionsShader = std::make_unique<Shader>(file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "ssr/ScreenSpaceReflections.frag");
    
    generateSkybox((file::texturePath() / "hdr/newport/NewportLoft.hdr").string(), glm::ivec2(512));
    // generateSkybox("", glm::ivec2(512));
    
    initFrameBuffers();
    initTextureRenderBuffers();
    
    glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
}

bool Renderer::debugMessageCallback(GLDEBUGPROC callback)
{
    int flags { 0 };  // Check to see if OpenGL debug context was set up correctly.
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(callback, nullptr);
    }
    
    return (flags & GL_CONTEXT_FLAG_DEBUG_BIT);
}

std::string Renderer::getVersion()
{
    return (reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

void Renderer::drawMesh(
    uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader,
    graphics::drawMode renderMode, const glm::mat4 &matrix,
    const graphics::DrawCallback &onDraw)
{
    if (shader.expired())
        WARN("A mesh was submitted with no shader. Was this intentional?");
    
    static GLenum drawModeToGLenum[] { GL_TRIANGLES, GL_LINES };
    GLenum mode = drawModeToGLenum[(int)renderMode];
    mRenderQueue.emplace_back(graphics::RenderQueueObject { vao, indicesCount, std::move(shader), mode, matrix, onDraw });
}

void Renderer::drawMesh(const SubMesh &subMesh, Material &material, const glm::mat4 &matrix)
{
    drawMesh(
        subMesh.vao(), subMesh.indicesCount(), material.shader(), material.drawMode(), matrix,
        [&]() { material.onDraw(); }
    );
}

void Renderer::drawMesh(const SharedMesh &mesh, const SharedMaterials &materials, const glm::mat4 &matrix)
{
    if (materials.size() == 1)
    {
        Material& material = *materials[0];
        
        for (const auto &subMesh : *mesh)
            drawMesh(*subMesh, material, matrix);
    }
    else
    {
        for (int i = 0; i < mesh->size(); ++i)
        {
            SubMesh& subMesh = *(*mesh)[i];
            Material& material = *materials[glm::min(i, static_cast<int>((materials.size() - 1)))];
            drawMesh(subMesh, material, matrix);
        }
    }
}

void Renderer::submit(const CameraSettings &cameraSettings)
{
    mCameraQueue.emplace_back(cameraSettings);
}

void Renderer::submit(const graphics::DirectionalLight &directionalLight)
{
    mDirectionalLightQueue.emplace_back(directionalLight);
}

void Renderer::submit(const graphics::PointLight &pointLight)
{
    mPointLightQueue.emplace_back(pointLight);
}

void Renderer::submit(const graphics::Spotlight &spotLight)
{
    mSpotlightQueue.emplace_back(spotLight);
}


void Renderer::render()
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Render Pass");
    if (window::bufferSize().x <= 0 || window::bufferSize().y <= 0)
        return;
    
    if (mCurrentRenderBufferSize != window::bufferSize())
    {
        detachTextureRenderBuffersFromFrameBuffers();
        initTextureRenderBuffers();
        mCurrentRenderBufferSize = window::bufferSize();
    }
    
    glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
    
    for (CameraSettings &camera : mCameraQueue)
    {
        mCurrentEV100 = camera.eV100;
        
        PROFILE_SCOPE_BEGIN(geometryTimer, "Geometry Pass");
        graphics::pushDebugGroup("Geometry Pass");
        
        mGeometryFramebuffer->bind();
        mGeometryFramebuffer->clear(camera.clearColour);
        
        const glm::mat4 cameraProjectionMatrix = glm::perspective(camera.fovY, window::aspectRatio(), camera.nearClipDistance, camera.farClipDistance);
        const glm::mat4 vpMatrix = cameraProjectionMatrix * camera.viewMatrix;
        
        for (const auto &rqo : mRenderQueue)
        {
            if (rqo.shader.expired())
                continue;
            
            const auto shader = rqo.shader.lock();
            shader->bind();
            shader->set("u_mvp_matrix", vpMatrix * rqo.matrix);
            shader->set("u_model_matrix", rqo.matrix);
            shader->set("u_camera_position_ws", glm::vec3(glm::inverse(camera.viewMatrix) * glm::vec4(glm::vec3(0.f), 1.f)));
            rqo.onDraw();
            glBindVertexArray(rqo.vao);
            glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
        }
        
        PROFILE_SCOPE_END(geometryTimer);
        graphics::popDebugGroup();
        
        directionalLightShadowMapping(camera);
        pointLightShadowMapping();
        spotlightShadowMapping();
        
        // Reset the viewport back to the normal size once we've finished rendering all the shadows.
        glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
        
        PROFILE_SCOPE_BEGIN(directionalLightTimer, "Directional Lighting");
        graphics::pushDebugGroup("Directional Lighting");
        
        mLightFramebuffer->bind();
        mLightFramebuffer->clear();
        const glm::vec3 cameraPosition = glm::inverse(camera.viewMatrix) * glm::vec4(glm::vec3(0.f), 1.f);
        
        mDirectionalLightShader->bind();

        mDirectionalLightShader->set("u_albedo_texture", mAlbedoTextureBuffer->getId(), 0);
        mDirectionalLightShader->set("u_position_texture", mPositionTextureBuffer->getId(), 1);
        mDirectionalLightShader->set("u_normal_texture", mNormalTextureBuffer->getId(), 2);
        mDirectionalLightShader->set("u_roughness_texture", mRoughnessTextureBuffer->getId(), 3);
        mDirectionalLightShader->set("u_metallic_texture", mMetallicTextureBuffer->getId(), 4);

        mDirectionalLightShader->set("u_camera_position_ws", cameraPosition);
        mDirectionalLightShader->set("u_view_matrix", camera.viewMatrix);


        glBindVertexArray(mFullscreenTriangle.vao());

        for (const graphics::DirectionalLight &directionalLight : mDirectionalLightQueue)
        {
            mDirectionalLightShader->set("u_cascade_distances", &(directionalLight.cascadeDepths[0]), static_cast<int>(directionalLight.cascadeDepths.size()));
            mDirectionalLightShader->set("u_cascade_count", static_cast<int>(directionalLight.cascadeDepths.size()));
            
            mDirectionalLightShader->set("u_bias", directionalLight.shadowBias);
            
            mDirectionalLightShader->set("u_light_direction", directionalLight.direction);
            mDirectionalLightShader->set("u_light_intensity", directionalLight.colourIntensity);
            mDirectionalLightShader->set("u_light_vp_matrix", &(directionalLight.vpMatrices[0]), static_cast<int>(directionalLight.vpMatrices.size()));
            mDirectionalLightShader->set("u_shadow_map_texture", directionalLight.shadowMap->getId(), 5);

            glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
        }
        
        PROFILE_SCOPE_END(directionalLightTimer);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(pointLightTimer, "Point Lighting");
        graphics::pushDebugGroup("Point Lighting");
        
        mPointLightShader->bind();
        
        mPointLightShader->set("u_albedo_texture", mAlbedoTextureBuffer->getId(), 0);
        mPointLightShader->set("u_position_texture", mPositionTextureBuffer->getId(), 1);
        mPointLightShader->set("u_normal_texture", mNormalTextureBuffer->getId(), 2);
        mPointLightShader->set("u_roughness_texture", mRoughnessTextureBuffer->getId(), 3);
        mPointLightShader->set("u_metallic_texture", mMetallicTextureBuffer->getId(), 4);
        
        mPointLightShader->set("u_camera_position_ws", cameraPosition);
        
        glBindVertexArray(mUnitSphere.vao());
        
        for (const auto &pointLight : mPointLightQueue)
        {
            const glm::mat4 pointLightModelMatrix = glm::translate(glm::mat4(1.f), pointLight.position) * glm::scale(glm::mat4(1.f), glm::vec3(pointLight.radius));
            mPointLightShader->set("u_mvp_matrix", vpMatrix * pointLightModelMatrix);
            
            mPointLightShader->set("u_light_position", pointLight.position);
            mPointLightShader->set("u_light_intensity", pointLight.colourIntensity);
            mPointLightShader->set("u_light_inv_sqr_radius", 1.f / (pointLight.radius * pointLight.radius));
            mPointLightShader->set("u_shadow_map_texture", pointLight.shadowMap->getId(), 5);
            mPointLightShader->set("u_z_far", pointLight.radius);
            mPointLightShader->set("u_softness_radius", pointLight.softnessRadius);
            mPointLightShader->set("u_bias", pointLight.bias);
            
            glDrawElements(GL_TRIANGLES, mUnitSphere.indicesCount(), GL_UNSIGNED_INT, nullptr);
        }
        
        
        PROFILE_SCOPE_END(pointLightTimer);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(spotLightTimer, "Spot Light");
        graphics::pushDebugGroup("Spot Light");
        
        mSpotlightShader->bind();
        
        mSpotlightShader->set("u_albedo_texture", mAlbedoTextureBuffer->getId(), 0);
        mSpotlightShader->set("u_position_texture", mPositionTextureBuffer->getId(), 1);
        mSpotlightShader->set("u_normal_texture", mNormalTextureBuffer->getId(), 2);
        mSpotlightShader->set("u_roughness_texture", mRoughnessTextureBuffer->getId(), 3);
        mSpotlightShader->set("u_metallic_texture", mMetallicTextureBuffer->getId(), 4);
        
        mSpotlightShader->set("u_camera_position_ws", cameraPosition);
        
        for (const graphics::Spotlight &spotLight : mSpotlightQueue)
        {
            mSpotlightShader->set("u_shadow_map_texture", spotLight.shadowMap->getId(), 5);
            
            mSpotlightShader->set("u_light_position", spotLight.position);
            mSpotlightShader->set("u_light_direction", spotLight.direction);
            const float lightAngleScale = 1.f / glm::max(0.001f, (spotLight.cosInnerAngle - spotLight.cosOuterAngle));
            const float lightAngleOffset = -spotLight.cosOuterAngle * lightAngleScale;
            mSpotlightShader->set("u_light_angle_scale", lightAngleScale);
            mSpotlightShader->set("u_light_angle_offset", lightAngleOffset);
            mSpotlightShader->set("u_light_inv_sqr_radius", 1.f / (spotLight.radius * spotLight.radius));
            mSpotlightShader->set("u_light_intensity", spotLight.colourIntensity);
            mSpotlightShader->set("u_light_vp_matrix", spotLight.vpMatrix);
            mSpotlightShader->set("u_z_far", spotLight.radius);
            mSpotlightShader->set("u_bias", spotLight.shadowBias);
            
            drawFullscreenTriangleNow();
        }
        
        PROFILE_SCOPE_END(spotLightTimer);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(iblTimer, "IBL Distant Light Probe");
        graphics::pushDebugGroup("IBL Distant Light Probe");
        
        // IBL ambient lighting. We already have the correct framebuffer bound.
        // mIblShader->bind();
        //
        // mIblShader->set("u_albedo_texture", mAlbedoTextureBuffer->getId(), 0);
        // mIblShader->set("u_position_texture", mPositionTextureBuffer->getId(), 1);
        // mIblShader->set("u_normal_texture", mNormalTextureBuffer->getId(), 2);
        // mIblShader->set("u_roughness_texture", mRoughnessTextureBuffer->getId(), 3);
        // mIblShader->set("u_metallic_texture", mMetallicTextureBuffer->getId(), 4);
        //
        // mIblShader->set("u_irradiance_texture", mIrradianceMap->getId(), 5);
        // mIblShader->set("u_pre_filter_texture", mPreFilterMap->getId(), 6);
        // mIblShader->set("u_brdf_lut_texture", mBrdfLutTextureBuffer->getId(), 7);
        //
        // mIblShader->set("u_camera_position_ws", cameraPosition);
        //
        // mIblShader->set("u_luminance_multiplier", mIblLuminanceMultiplier);
        //
        // drawFullscreenTriangleNow();
        
        PROFILE_SCOPE_END(iblTimer);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(screenSpace, "Reflections");
        graphics::pushDebugGroup("Reflections");
        
        mReflectionFramebuffer->bind();
        mReflectionFramebuffer->clear();
        mScreenSpaceReflectionsShader->bind();
        
        mScreenSpaceReflectionsShader->set("u_positionTexture", mPositionTextureBuffer->getId(), 0);
        mScreenSpaceReflectionsShader->set("u_normalTexture", mNormalTextureBuffer->getId(), 1);
        mScreenSpaceReflectionsShader->set("u_depthTexture", mDepthTextureBuffer->getId(), 2);
        mScreenSpaceReflectionsShader->set("u_colourTexture", mLightTextureBuffer->getId(), 3);
        
        const float maxLuminance = 1.2f * glm::pow(2.f, mCurrentEV100);
        const float exposure = 1.f / maxLuminance;
        mScreenSpaceReflectionsShader->set("u_exposure", exposure);
        mScreenSpaceReflectionsShader->set("u_cameraPositionWs", cameraPosition);
        mScreenSpaceReflectionsShader->set("u_vpMatrix", vpMatrix);
        
        drawFullscreenTriangleNow();
        
        PROFILE_SCOPE_END(screenSpace);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(deferredTimer, "Skybox Pass");
        graphics::pushDebugGroup("Combine + Skybox Pass");
        
        // Deferred Lighting step.
        
        mDeferredLightFramebuffer->bind();
        mDeferredLightFramebuffer->clear(glm::vec4(glm::vec3(0.f), 0.f));
        
        mDeferredLightShader->bind();
        
        const glm::mat4 v = glm::mat4(glm::mat3(camera.viewMatrix));
        const glm::mat4 vp = cameraProjectionMatrix * v;
        const glm::mat4 ivp = glm::inverse(vp);
        
        mDeferredLightShader->set("u_irradiance_texture", mLightTextureBuffer->getId(), 0);
        mDeferredLightShader->set("u_emissive_texture", mEmissiveTextureBuffer->getId(), 1);
        mDeferredLightShader->set("u_depth_texture", mDepthTextureBuffer->getId(), 2);
        mDeferredLightShader->set("u_skybox_texture", mHdrSkybox->getId(), 3);
        
        mDeferredLightShader->set("u_inverse_vp_matrix", ivp);
        mDeferredLightShader->set("u_luminance_multiplier", mIblLuminanceMultiplier);
        
        drawFullscreenTriangleNow();
        PROFILE_SCOPE_END(deferredTimer);
        graphics::popDebugGroup();
        
        PROFILE_SCOPE_BEGIN(postProcessTimer, "Post-processing Stack");
        graphics::pushDebugGroup("Post-processing Pass");
        
        graphics::copyTexture2D(*mDeferredLightingTextureBuffer, *mPrimaryImageBuffer);
        for (std::unique_ptr<PostProcessLayer> &postProcessLayer : camera.postProcessStack)
        {
            postProcessLayer->draw(mPrimaryImageBuffer.get(), mAuxiliaryImageBuffer.get());
            graphics::copyTexture2D(*mAuxiliaryImageBuffer, *mPrimaryImageBuffer); // Yes this is bad. I'm lazy.
        }
        
        PROFILE_SCOPE_END(postProcessTimer);
        graphics::popDebugGroup();
    }
    
    graphics::popDebugGroup();
}

void Renderer::directionalLightShadowMapping(const CameraSettings &cameraSettings)
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Directional Light Shadow Mapping");
    const auto resize = [](const glm::vec4 &v) { return v / v.w; };
    
    mShadowFramebuffer->bind();
    mDirectionalLightShadowShader->bind();
    
    for (graphics::DirectionalLight &directionalLight : mDirectionalLightQueue)
    {
        PROFILE_SCOPE_BEGIN(lightTimer, "Directional Light Shadow Mapping");
        directionalLight.cascadeDepths.clear();
        directionalLight.cascadeDepths.reserve(directionalLight.shadowCascadeMultipliers.size());
        for (const auto &multiplier : directionalLight.shadowCascadeMultipliers)
            directionalLight.cascadeDepths.emplace_back(cameraSettings.farClipDistance * multiplier);
        
        graphics::pushDebugGroup("Directional Light");
        
        const glm::ivec2 &shadowMapSize = directionalLight.shadowMap->getSize();
        glViewport(0, 0, shadowMapSize.x, shadowMapSize.y);
        
        std::vector<float> depths { cameraSettings.nearClipDistance };
        for (const float &depth : directionalLight.cascadeDepths)
            depths.emplace_back(depth);
        depths.emplace_back(cameraSettings.farClipDistance);
        
        for (int j = 0; j < directionalLight.shadowMap->getLayerCount(); ++j)
        {
            PROFILE_SCOPE_BEGIN(shadowCreation, "Cascade Pass");
            graphics::pushDebugGroup("Cascade Pass");
            
            mShadowFramebuffer->attachDepthBuffer(*directionalLight.shadowMap, j);
            mShadowFramebuffer->clearDepthBuffer();
            
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
            
            PROFILE_SCOPE_BEGIN(rqoTimer, "Command Upload");
            for (const auto &rqo : mRenderQueue)
            {
                const glm::mat4 &modelMatrix = rqo.matrix;
                const glm::mat4 mvp = lightProjectionMatrix * lightViewMatrix * modelMatrix;
                mDirectionalLightShadowShader->set("u_mvp_matrix", mvp);
                glBindVertexArray(rqo.vao);
                glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
            }
            PROFILE_SCOPE_END(rqoTimer);
            
            mShadowFramebuffer->detachDepthBuffer();
            PROFILE_SCOPE_END(shadowCreation);
            graphics::popDebugGroup();
        }
        
        graphics::popDebugGroup();
        PROFILE_SCOPE_END(lightTimer);
    }
    
    graphics::popDebugGroup();
}

void Renderer::pointLightShadowMapping()
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Point Light Shadow Mapping");
    mShadowFramebuffer->bind();
    mPointLightShadowShader->bind();
    
    for (auto &pointLight : mPointLightQueue)
    {
        PROFILE_SCOPE_BEGIN(pointLightTimer, "Point Light Shadow Pass");
        graphics::pushDebugGroup("Point Light Pass");
        const glm::ivec2 size = pointLight.shadowMap->getSize();
        glViewport(0, 0, size.x, size.y);
        
        mPointLightShadowShader->set("u_light_pos", pointLight.position);
        mPointLightShadowShader->set("u_z_far", pointLight.radius);
        
        for (int viewIndex = 0; viewIndex < 6; ++viewIndex)
        {
            PROFILE_SCOPE_BEGIN(faceTimer, "View Pass");
            graphics::pushDebugGroup("Rendering Face");
            Cubemap &shadowMap = *pointLight.shadowMap;
            mShadowFramebuffer->attachDepthBuffer(shadowMap, viewIndex, 0);
            mShadowFramebuffer->clearDepthBuffer();
            
            for (const auto &rqo : mRenderQueue)
            {
                const glm::mat4 &modelMatrix = rqo.matrix;
                const glm::mat4 mvp = pointLight.vpMatrices[viewIndex] * modelMatrix;
                mPointLightShadowShader->set("u_model_matrix", modelMatrix);
                mPointLightShadowShader->set("u_mvp_matrix", mvp);
                
                glBindVertexArray(rqo.vao);
                glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
            }
            
            mShadowFramebuffer->detachDepthBuffer();
            graphics::popDebugGroup();
        }
        PROFILE_SCOPE_END(pointLightTimer);
        graphics::popDebugGroup();
    }
    graphics::popDebugGroup();
}

void Renderer::spotlightShadowMapping()
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Spotlight Shadow Mapping");
    
    mSpotlightShadowShader->bind();
    mShadowFramebuffer->bind();
    
    for (const graphics::Spotlight &spotlight : mSpotlightQueue)
    {
        glm::ivec2 size = spotlight.shadowMap->getSize();
        glViewport(0, 0, size.x, size.y);
        
        mShadowFramebuffer->attachDepthBuffer(spotlight.shadowMap.get());
        mShadowFramebuffer->clearDepthBuffer();
        
        const glm::mat4 &vpMatrix = spotlight.vpMatrix;
        mSpotlightShadowShader->set("u_light_pos", spotlight.position);
        mSpotlightShadowShader->set("u_z_far", spotlight.radius);
        
        for (const auto &rqo : mRenderQueue)
        {
            const glm::mat4 mvpMatrix = vpMatrix * rqo.matrix;
            mSpotlightShadowShader->set("u_mvp_matrix", mvpMatrix);
            mSpotlightShadowShader->set("u_model_matrix", rqo.matrix);
            
            glBindVertexArray(rqo.vao);
            glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
        }
        
        mShadowFramebuffer->detachDepthBuffer();
    }
    
    graphics::popDebugGroup();
}

void Renderer::clear()
{
    uint64_t renderQueueCount = mRenderQueue.size();
    mRenderQueue.clear();
    mRenderQueue.reserve(renderQueueCount);
    
    uint64_t cameraCount = mCameraQueue.size();
    mCameraQueue.clear();
    mCameraQueue.reserve(cameraCount);
    
    uint64_t directionalLightCount = mDirectionalLightQueue.size();
    mDirectionalLightQueue.clear();
    mDirectionalLightQueue.reserve(directionalLightCount);
    
    uint64_t pointLightCount = mPointLightQueue.size();
    mPointLightQueue.clear();
    mPointLightQueue.reserve(pointLightCount);
    
    uint64_t spotLightCount = mSpotlightQueue.size();
    mSpotlightQueue.clear();
    mSpotlightQueue.reserve(spotLightCount);
}

void Renderer::initFrameBuffers()
{
    // One, Zero (override any geometry that is further away from the camera).
    mGeometryFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);
    
    // One, One (additive blending for each light that we pass through)
    mLightFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ONE, GL_ALWAYS);
    
    // We only ever write to this framebuffer once, so it shouldn't matter.
    mDeferredLightFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ONE, GL_ALWAYS);
    
    mReflectionFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ONE, GL_ALWAYS);
    
    mShadowFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);
}

void Renderer::initTextureRenderBuffers()
{
    mPositionTextureBuffer           = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mAlbedoTextureBuffer             = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mNormalTextureBuffer             = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16_SNORM,           GL_NEAREST, GL_NEAREST);
    mEmissiveTextureBuffer           = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mRoughnessTextureBuffer          = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_R16,                   GL_NEAREST, GL_NEAREST);
    mMetallicTextureBuffer           = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_R16,                   GL_NEAREST, GL_NEAREST);
    mLightTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mDeferredLightingTextureBuffer   = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mShadowTextureBuffer             = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_R16F,                  GL_NEAREST, GL_NEAREST);
    mDepthTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_DEPTH_COMPONENT32F,    GL_NEAREST, GL_NEAREST);
    mPrimaryImageBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mAuxiliaryImageBuffer            = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mReflectionImageBuffer           = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_LINEAR,  GL_LINEAR);
    
    // Make sure that the framebuffers have been set up before calling this function.
    mGeometryFramebuffer->attach(mPositionTextureBuffer.get(),    0);
    mGeometryFramebuffer->attach(mNormalTextureBuffer.get(),      1);
    mGeometryFramebuffer->attach(mAlbedoTextureBuffer.get(),      2);
    mGeometryFramebuffer->attach(mEmissiveTextureBuffer.get(),    3);
    mGeometryFramebuffer->attach(mRoughnessTextureBuffer.get(),   4);
    mGeometryFramebuffer->attach(mMetallicTextureBuffer.get(),    5);
    
    mGeometryFramebuffer->attachDepthBuffer(mDepthTextureBuffer.get());
    
    // Lighting.
    mLightFramebuffer->attach(mLightTextureBuffer.get(), 0);
    mLightFramebuffer->attach(mShadowTextureBuffer.get(), 1);
    
    // Reflection Buffer
    mReflectionFramebuffer->attach(mReflectionImageBuffer.get(), 0);
    
    // Deferred Lighting.
    mDeferredLightFramebuffer->attach(mDeferredLightingTextureBuffer.get(), 0);
}

void Renderer::detachTextureRenderBuffersFromFrameBuffers()
{
    mGeometryFramebuffer->detach(0);
    mGeometryFramebuffer->detach(1);
    mGeometryFramebuffer->detach(2);
    mGeometryFramebuffer->detach(3);
    mGeometryFramebuffer->detach(4);
    mGeometryFramebuffer->detach(5);
    mGeometryFramebuffer->detachDepthBuffer();
    
    mLightFramebuffer->detach(0);
    mLightFramebuffer->detach(1);
    
    mReflectionFramebuffer->detach(0);
    
    mDeferredLightFramebuffer->detach(0);
}

std::unique_ptr<Cubemap> Renderer::createCubemapFromHdrTexture(HdrTexture *hdrTexture, const glm::ivec2 &size)
{
    auto cubemap = std::make_unique<Cubemap>(size, GL_RGB16F);
    
    FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);
    
    const glm::mat4 views[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    auxiliaryFrameBuffer.bind();
    mHdrToCubemapShader->bind();
    mHdrToCubemapShader->set("u_texture", hdrTexture->getId(), 0);
    
    glViewport(0, 0, size.x, size.y);
    
    glBindVertexArray(mFullscreenTriangle.vao());
    
    for (int i = 0; i < 6; ++i)
    {
        mHdrToCubemapShader->set("u_view_matrix", views[i]);
        auxiliaryFrameBuffer.attach(cubemap.get(), 0, i);
        auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));
        
        glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
        
        auxiliaryFrameBuffer.detach(0);
    }
    
    return cubemap;
}

std::unique_ptr<Cubemap> Renderer::generateIrradianceMap(Cubemap *cubemap, const glm::ivec2 &size)
{
    auto irradiance = std::make_unique<Cubemap>(size, GL_RGB16F);
    
    FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);
    
    const glm::mat4 views[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    auxiliaryFrameBuffer.bind();
    mCubemapToIrradianceShader->bind();
    mCubemapToIrradianceShader->set("u_environment_texture", cubemap->getId(), 0);
    
    glViewport(0, 0, size.x, size.y);
    
    glBindVertexArray(mFullscreenTriangle.vao());
    
    for (int i = 0; i < 6; ++i)
    {
        mCubemapToIrradianceShader->set("u_view_matrix", views[i]);
        auxiliaryFrameBuffer.attach(irradiance.get(), 0, i);
        auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));
        
        glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
        
        auxiliaryFrameBuffer.detach(0);
    }
    
    return irradiance;
}

std::unique_ptr<Cubemap> Renderer::generatePreFilterMap(Cubemap *cubemap, const glm::ivec2 &size)
{
    const int maxMipLevels = 5;
    auto filter = std::make_unique<Cubemap>(size, GL_RGB16F, maxMipLevels);
    
    FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);
    
    const glm::mat4 views[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    auxiliaryFrameBuffer.bind();
    mPreFilterShader->bind();
    mPreFilterShader->set("u_environment_texture", cubemap->getId(), 0);
    
    glBindVertexArray(mFullscreenTriangle.vao());
    
    for (int mip = 0; mip < maxMipLevels; ++mip)
    {
        const glm::ivec2 mipSize = size >> mip;
        glViewport(0, 0, mipSize.x, mipSize.y);
        
        const float roughness = static_cast<float>(mip) / (static_cast<float>(maxMipLevels) - 1.f);
        mPreFilterShader->set("u_roughness", roughness);
        
        for (int i = 0; i < 6; ++i)
        {
            mPreFilterShader->set("u_view_matrix", views[i]);
            auxiliaryFrameBuffer.attach(filter.get(), 0, i, mip);
            auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));
            
            glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
            
            auxiliaryFrameBuffer.detach(0);
        }
    }
    
    return filter;
}

std::unique_ptr<TextureBufferObject> Renderer::generateBrdfLut(const glm::ivec2 &size)
{
    auto lut = std::make_unique<TextureBufferObject>(size, GL_RGB16F, graphics::filter::Linear, graphics::wrap::ClampToEdge);
    
    FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);
    
    auxiliaryFrameBuffer.bind();
    mIntegrateBrdfShader->bind();
    auxiliaryFrameBuffer.attach(lut.get(), 0);
    glViewport(0, 0, size.x, size.y);
    auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));
    drawFullscreenTriangleNow();
    auxiliaryFrameBuffer.detach(0);
    
    return lut;
}


void Renderer::generateSkybox(std::string_view path, const glm::ivec2 desiredSize)
{
    mHdrImage = std::make_unique<HdrTexture>(path);
    mHdrImage->setDebugName("HDRSkyboxImage");
    mHdrSkybox = createCubemapFromHdrTexture(mHdrImage.get(), desiredSize);
    mHdrSkybox->setDebugName("Skybox");
    mIrradianceMap = generateIrradianceMap(mHdrSkybox.get(), desiredSize / 8);  // 8
    mIrradianceMap->setDebugName("Irradiance");
    mPreFilterMap = generatePreFilterMap(mHdrSkybox.get(), desiredSize / 4);  // 4
    mPreFilterMap->setDebugName("Prefilter Map");
    mBrdfLutTextureBuffer = generateBrdfLut(desiredSize);
}

void Renderer::drawFullscreenTriangleNow()
{
    glBindVertexArray(mFullscreenTriangle.vao());
    glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
}

const TextureBufferObject &Renderer::getPrimaryBuffer()
{
    return *mPrimaryImageBuffer;
}

const TextureBufferObject &Renderer::getAlbedoBuffer()
{
    return *mAlbedoTextureBuffer;
}

const TextureBufferObject &Renderer::getNormalBuffer()
{
    return *mNormalTextureBuffer;
}

const TextureBufferObject &Renderer::getPositionBuffer()
{
    return *mPositionTextureBuffer;
}

const TextureBufferObject &Renderer::getEmissiveBuffer()
{
    return *mEmissiveTextureBuffer;
}

const TextureBufferObject &Renderer::getLightBuffer()
{
    return *mLightTextureBuffer;
}

const TextureBufferObject &Renderer::getDepthBuffer()
{
    return *mDepthTextureBuffer;
}

const TextureBufferObject &Renderer::getShadowBuffer()
{
    return *mShadowTextureBuffer;
}

const TextureBufferObject &Renderer::getRoughnessBuffer()
{
    return *mRoughnessTextureBuffer;
}

const TextureBufferObject &Renderer::getMetallicBuffer()
{
    return *mMetallicTextureBuffer;
}

const TextureBufferObject &Renderer::getDeferredLightingBuffer()
{
    return *mDeferredLightingTextureBuffer;
}

std::vector<graphics::DirectionalLight> &Renderer::getDirectionalLights()
{
    return mDirectionalLightQueue;
}

const TextureBufferObject &Renderer::getReflectionBuffer()
{
    return *mReflectionImageBuffer;
}

float Renderer::getCurrentEV100() const
{
    return mCurrentEV100;
}

void Renderer::setIblMultiplier(float m)
{
    mIblLuminanceMultiplier = glm::abs(m);
}

void Renderer::rendererGuiNewFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

