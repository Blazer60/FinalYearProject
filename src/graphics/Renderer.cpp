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
    mCurrentRenderBufferSize(window::bufferSize()),
    mFullscreenTriangle(primitives::fullscreenTriangle()),
    mUnitSphere(primitives::invertedSphere()),
    mLine(primitives::line())
{
    // Blending texture data / enabling lerping.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    graphics::pushDebugGroup("Setup");

    auto fsTriShader = file::shaderPath() / "FullscreenTriangle.vert";
    mDirectionalLightShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "lighting/DirectionalLight.frag");
    mPointLightShader = std::make_unique<Shader>(file::shaderPath() / "lighting/PointLight.vert", file::shaderPath() / "lighting/PointLight.frag");
    mSpotlightShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "lighting/SpotLight.frag");
    mIblShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "lighting/IBL.frag");
    mDeferredLightShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "lighting/CombineOutput.frag");
    mDirectionalLightShadowShader = std::make_unique<Shader>(file::shaderPath() / "shadow/Shadow.vert", file::shaderPath() / "shadow/Shadow.frag");
    mPointLightShadowShader = std::make_unique<Shader>(file::shaderPath() / "shadow/PointShadow.vert", file::shaderPath() / "shadow/PointShadow.frag");
    mSpotlightShadowShader = std::make_unique<Shader>(file::shaderPath() / "shadow/PointShadow.vert", file::shaderPath() / "shadow/PointShadow.frag");
    mHdrToCubemapShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "cubemap/ToCubemap.frag");
    mCubemapToIrradianceShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "cubemap/IrradianceMap.frag");
    mPreFilterShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() /  "cubemap/PreFilter.frag");
    mIntegrateBrdfShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "brdf/IntegrateBrdf.frag");
    mScreenSpaceReflectionsShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "ssr/SsrDda.frag");
    mColourResolveShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "ssr/ColourResolve.frag");
    mBlurShader = std::make_unique<Shader>(fsTriShader, file::shaderPath() / "postProcessing/bloom/BloomDownSample.frag");
    mDebugShader = std::make_unique<Shader>(file::shaderPath() / "geometry/debug/Debug.vert", file::shaderPath() / "geometry/debug/Debug.frag");
    mLineShader = std::make_unique<Shader>(file::shaderPath() / "geometry/debug/Line.vert", file::shaderPath() / "geometry/debug/Line.frag");
    
    generateSkybox((file::texturePath() / "hdr/newport/NewportLoft.hdr").string(), glm::ivec2(512));

    initFrameBuffers();
    initTextureRenderBuffers();
    
    setViewportSize();
    graphics::popDebugGroup();
}

void Renderer::initFrameBuffers()
{
    // One, Zero (override any geometry that is further away from the camera).
    mGeometryFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);
    
    // One, One (additive blending for each light that we pass through)
    mLightFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ONE, GL_ALWAYS);
    
    // We only ever write to this framebuffer once, so it shouldn't matter.
    mDeferredLightFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ONE, GL_ALWAYS);
    
    mSsrFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_ALWAYS);

    mReflectionFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_ALWAYS);

    mShadowFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);

    mBlurFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_ALWAYS);

    mDebugFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);
}

void Renderer::initTextureRenderBuffers()
{
    auto ssrSize = glm::ivec2(glm::vec2(window::bufferSize()) * glm::vec2(1.f / mReflectionDivideSize));
    mPositionTextureBuffer           = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mAlbedoTextureBuffer             = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mNormalTextureBuffer             = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16_SNORM,           GL_NEAREST, GL_NEAREST);
    mEmissiveTextureBuffer           = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mRoughnessTextureBuffer          = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_R16,                   GL_NEAREST, GL_NEAREST);
    mMetallicTextureBuffer           = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_R16,                   GL_NEAREST, GL_NEAREST);
    mLightTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                graphics::filter::Nearest, graphics::wrap::ClampToEdge);
    mDeferredLightingTextureBuffer   = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                graphics::filter::LinearMipmapLinear, graphics::wrap::ClampToEdge, 8);
    mDepthTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_DEPTH_COMPONENT32F,    graphics::filter::Nearest, graphics::wrap::ClampToBorder);
    mPrimaryImageBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mAuxiliaryImageBuffer            = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGB16F,                GL_NEAREST, GL_NEAREST);
    mSsrDataTextureBuffer            = std::make_unique<TextureBufferObject>(ssrSize,              GL_RGBA16F,               graphics::filter::Nearest, graphics::wrap::ClampToEdge);
    mReflectionTextureBuffer         = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F,               graphics::filter::Linear,  graphics::wrap::ClampToEdge);
    mDebugTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16,                graphics::filter::Linear,  graphics::wrap::ClampToEdge);
    
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

    // Reflection Buffer
    mSsrFramebuffer->attach(mSsrDataTextureBuffer.get(), 0);

    mReflectionFramebuffer->attach(mReflectionTextureBuffer.get(), 0);
    
    // Deferred Lighting.
    mDeferredLightFramebuffer->attach(mDeferredLightingTextureBuffer.get(), 0);

    // Debug.
    mDebugFramebuffer->attach(mDebugTextureBuffer.get(), 0);
}

void Renderer::detachTextureRenderBuffersFromFrameBuffers() const
{
    mGeometryFramebuffer->detach(0);
    mGeometryFramebuffer->detach(1);
    mGeometryFramebuffer->detach(2);
    mGeometryFramebuffer->detach(3);
    mGeometryFramebuffer->detach(4);
    mGeometryFramebuffer->detach(5);
    mGeometryFramebuffer->detachDepthBuffer();
    
    mLightFramebuffer->detach(0);

    mSsrFramebuffer->detach(0);

    mReflectionFramebuffer->detach(0);

    mDeferredLightFramebuffer->detach(0);

    mDebugFramebuffer->detach(0);
}

bool Renderer::debugMessageCallback(GLDEBUGPROC callback)
{
    int flags { 0 };  // Check to see if OpenGL debug context was set up correctly.
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    
    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(callback, nullptr);
    }
    
    return (flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0;
}

std::string Renderer::getVersion()
{
    return (reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

void Renderer::drawMesh(
    const uint32_t vao, const int32_t indicesCount, std::weak_ptr<Shader> shader,
    graphics::drawMode renderMode, const glm::mat4 &matrix,
    const graphics::DrawCallback &onDraw)
{
    if (shader.expired())
        WARN("A mesh was submitted with no shader. Was this intentional?");
    
    static GLenum drawModeToGLenum[] { GL_TRIANGLES, GL_LINES };
    const GLenum mode = drawModeToGLenum[static_cast<int>(renderMode)];
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

void Renderer::drawDebugMesh(const uint32_t vao, const int32_t indicesCount, const glm::mat4& matrix, const glm::vec3& colour)
{
    mDebugQueue.emplace_back(graphics::DebugQueueObject { vao, indicesCount, matrix, colour });
}

void Renderer::drawDebugMesh(const SubMesh& subMesh, const glm::mat4& matrix, const glm::vec3& colour)
{
    drawDebugMesh(subMesh.vao(), subMesh.indicesCount(), matrix, colour);
}

void Renderer::drawDebugMesh(const SharedMesh& mesh, const glm::mat4& matrix, const glm::vec3& colour)
{
    for (int i = 0; i < mesh->size(); ++i)
    {
        SubMesh &subMesh = *(*mesh)[i];
        drawDebugMesh(subMesh, matrix, colour);
    }
}

void Renderer::drawDebugLine(const glm::vec3& startPosition, const glm::vec3& endPosition, const glm::vec3& colour)
{
    mLineQueue.emplace_back(graphics::LineQueueObject { startPosition, endPosition, colour });
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
    
    setViewportSize(window::bufferSize());
    
    for (CameraSettings &camera : mCameraQueue)
    {
        mCurrentEV100 = camera.eV100;
        const float exposure = getCurrentExposure();
        
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
        mDirectionalLightShader->set("u_exposure", exposure);

        glBindVertexArray(mFullscreenTriangle.vao());

        for (const graphics::DirectionalLight &directionalLight : mDirectionalLightQueue)
        {
            mDirectionalLightShader->set("u_cascade_distances", directionalLight.cascadeDepths.data(), static_cast<int>(directionalLight.cascadeDepths.size()));
            mDirectionalLightShader->set("u_cascade_count", static_cast<int>(directionalLight.cascadeDepths.size()));
            
            mDirectionalLightShader->set("u_bias", directionalLight.shadowBias);
            
            mDirectionalLightShader->set("u_light_direction", directionalLight.direction);
            mDirectionalLightShader->set("u_light_intensity", directionalLight.colourIntensity);
            mDirectionalLightShader->set("u_light_vp_matrix", directionalLight.vpMatrices.data(), static_cast<int>(directionalLight.vpMatrices.size()));
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
        mPointLightShader->set("u_exposure", exposure);
        
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
        mSpotlightShader->set("u_exposure", exposure);
        
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
        PROFILE_SCOPE_BEGIN(screenSpace, "Reflections");
        graphics::pushDebugGroup("Reflections");
        
        // Render at half-resolution for performance.
        glViewport(0, 0, mSsrDataTextureBuffer->getSize().x, mSsrDataTextureBuffer->getSize().y);
        mSsrFramebuffer->bind();
        mSsrFramebuffer->clear(glm::vec4(0.f));
        mScreenSpaceReflectionsShader->bind();

        mDepthTextureBuffer->setBorderColour(glm::vec4(0.f, 0.f, 0.f, 1.f));

        mScreenSpaceReflectionsShader->set("u_positionTexture", mPositionTextureBuffer->getId(), 0);
        mScreenSpaceReflectionsShader->set("u_normalTexture", mNormalTextureBuffer->getId(), 1);
        mScreenSpaceReflectionsShader->set("u_depthTexture", mDepthTextureBuffer->getId(), 2);
        mScreenSpaceReflectionsShader->set("u_roughnessTexture", mRoughnessTextureBuffer->getId(), 3);

        mScreenSpaceReflectionsShader->set("u_viewMatrix", camera.viewMatrix);
        mScreenSpaceReflectionsShader->set("u_cameraPosition", cameraPosition);


        const glm::mat4 scaleTextureSpace = glm::scale(glm::mat4(1.f), glm::vec3(0.5f, 0.5f, 1.f));
        const glm::mat4 translateTextureSpace = glm::translate(glm::mat4(1.f), glm::vec3(0.5f, 0.5f, 0.f));
        const glm::mat4 scaleFrameBufferSpace = glm::scale(glm::mat4(1.f), glm::vec3(mLightTextureBuffer->getSize().x, mLightTextureBuffer->getSize().y, 1));

        const glm::mat4 viewToPixelCoordMatrix = scaleFrameBufferSpace * translateTextureSpace * scaleTextureSpace * cameraProjectionMatrix;
        mScreenSpaceReflectionsShader->set("u_proj", viewToPixelCoordMatrix);

        mScreenSpaceReflectionsShader->set("u_nearPlaneZ", -camera.nearClipDistance);

        drawFullscreenTriangleNow();

        glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);

        mReflectionFramebuffer->bind();
        mReflectionFramebuffer->clear(glm::vec4(0.f));
        mColourResolveShader->bind();

        mColourResolveShader->set("u_albedoTexture",             mAlbedoTextureBuffer->getId(),     0);
        mColourResolveShader->set("u_positionTexture",           mPositionTextureBuffer->getId(),   1);
        mColourResolveShader->set("u_normalTexture",             mNormalTextureBuffer->getId(),     2);
        mColourResolveShader->set("u_roughnessTexture",          mRoughnessTextureBuffer->getId(),  3);
        mColourResolveShader->set("u_metallicTexture",           mMetallicTextureBuffer->getId(),   4);
        mColourResolveShader->set("u_reflectionDataTexture",     mSsrDataTextureBuffer->getId(),    5);
        mColourResolveShader->set("u_colourTexture",             mDeferredLightingTextureBuffer->getId(), 6);
        mColourResolveShader->set("u_depthTexture",              mDepthTextureBuffer->getId(),      7);
        mColourResolveShader->set("u_irradianceTexture",         mIrradianceMap->getId(),           8);
        mColourResolveShader->set("u_pre_filterTexture",         mPreFilterMap->getId(),            9);
        mColourResolveShader->set("u_brdfLutTexture",            mBrdfLutTextureBuffer->getId(),    10);
        mColourResolveShader->set("u_emissiveTexture", mEmissiveTextureBuffer->getId(), 11);

        mColourResolveShader->set("u_cameraPositionWs", cameraPosition);
        mColourResolveShader->set("u_exposure", exposure);
        mColourResolveShader->set("u_colour_max_lod", static_cast<int>(mDeferredLightingTextureBuffer->getMipLevels()));
        mColourResolveShader->set("u_luminance_multiplier", mIblLuminanceMultiplier);
        mColourResolveShader->set("u_maxDistanceFalloff", mRoughnessFallOff);

        drawFullscreenTriangleNow();

        glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
        
        PROFILE_SCOPE_END(screenSpace);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(deferredTimer, "Skybox Pass");
        graphics::pushDebugGroup("Combine + Skybox Pass");
        
        // Deferred Lighting step.
        
        mDeferredLightFramebuffer->bind();
        mDeferredLightFramebuffer->clear(glm::vec4(glm::vec3(0.f), 0.f));
        
        mDeferredLightShader->bind();
        
        const glm::mat4 viewMatrixNoPosition = glm::mat4(glm::mat3(camera.viewMatrix));
        const glm::mat4 inverseViewProjection = glm::inverse(cameraProjectionMatrix * viewMatrixNoPosition);

        mDeferredLightShader->set("u_irradiance_texture", mLightTextureBuffer->getId(), 0);
        mDeferredLightShader->set("u_emissive_texture", mEmissiveTextureBuffer->getId(), 1);
        mDeferredLightShader->set("u_depth_texture", mDepthTextureBuffer->getId(), 2);
        mDeferredLightShader->set("u_skybox_texture", mHdrSkybox->getId(), 3);
        mDeferredLightShader->set("u_reflection_texture", mReflectionTextureBuffer->getId(), 4);

        mDeferredLightShader->set("u_inverse_vp_matrix", inverseViewProjection);
        mDeferredLightShader->set("u_luminance_multiplier", mIblLuminanceMultiplier);
        mDeferredLightShader->set("u_exposure", exposure);
        
        drawFullscreenTriangleNow();

        blurTexture(*mDeferredLightingTextureBuffer);

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

        PROFILE_SCOPE_BEGIN(debugView, "Debug View");
        graphics::pushDebugGroup("Debug View");

        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        mDebugFramebuffer->bind();
        mDebugFramebuffer->clear(glm::vec4(0.f));
        mDebugShader->bind();

        for (const auto & [vao, count, modelMatrix, colour] : mDebugQueue)
        {
            mDebugShader->set("u_mvp_matrix", vpMatrix * modelMatrix);
            mDebugShader->set("u_colour", colour);
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        }

        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        mLineShader->bind();
        mLineShader->set("u_mvp_matrix", vpMatrix);
        glBindVertexArray(mLine.vao());
        for (const auto &[startPosition, endPosition, colour] : mLineQueue)
        {
            mLineShader->set("u_locationA", startPosition);
            mLineShader->set("u_locationB", endPosition);
            mLineShader->set("u_colour",    colour);
            glDrawElements(GL_LINES, mLine.indicesCount(), GL_UNSIGNED_INT, nullptr);
        }

        PROFILE_SCOPE_END(debugView);
        graphics::popDebugGroup();
    }

    graphics::popDebugGroup();
}

void Renderer::directionalLightShadowMapping(const CameraSettings &cameraSettings)
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Directional Light Shadow Mapping");
    const auto resize = [](const glm::vec4 &vec) { return vec / vec.w; };
    
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
        
        std::vector depths { cameraSettings.nearClipDistance };
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

void Renderer::pointLightShadowMapping() const
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

void Renderer::spotlightShadowMapping() const
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Spotlight Shadow Mapping");
    
    mSpotlightShadowShader->bind();
    mShadowFramebuffer->bind();
    
    for (const graphics::Spotlight &spotlight : mSpotlightQueue)
    {
        const glm::ivec2 size = spotlight.shadowMap->getSize();
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

void Renderer::blurTexture(const TextureBufferObject& texture) const
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Auxilliary Blur Pass");
    const uint32_t mipLevels = texture.getMipLevels();
    const glm::ivec2 &size = texture.getSize();
    mBlurFramebuffer->bind();
    mBlurShader->bind();
    mBlurShader->set("u_texture", texture.getId(), 0);

    for (int i = 1; i < mipLevels; ++i)
    {
        glViewport(0, 0, size.x >> i, size.y >> i);
        mBlurShader->set("u_mip_level", i - 1);
        mBlurFramebuffer->attach(&texture, 0, i);
        drawFullscreenTriangleNow();
        mBlurFramebuffer->detach(0);
    }

    glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
    graphics::popDebugGroup();
}

void Renderer::setViewportSize(const glm::ivec2& size)
{
    glViewport(0, 0, size.x, size.y);
}

void Renderer::clear()
{
    const uint64_t renderQueueCount = mRenderQueue.size();
    mRenderQueue.clear();
    mRenderQueue.reserve(renderQueueCount);

    const uint64_t cameraCount = mCameraQueue.size();
    mCameraQueue.clear();
    mCameraQueue.reserve(cameraCount);

    const uint64_t directionalLightCount = mDirectionalLightQueue.size();
    mDirectionalLightQueue.clear();
    mDirectionalLightQueue.reserve(directionalLightCount);

    const uint64_t pointLightCount = mPointLightQueue.size();
    mPointLightQueue.clear();
    mPointLightQueue.reserve(pointLightCount);

    const uint64_t spotLightCount = mSpotlightQueue.size();
    mSpotlightQueue.clear();
    mSpotlightQueue.reserve(spotLightCount);

    const uint64_t debugQueueCount = mDebugQueue.size();
    mDebugQueue.clear();
    mDebugQueue.reserve(debugQueueCount);

    const uint64_t lineQueueCount = mLineQueue.size();
    mLineQueue.clear();
    mLineQueue.reserve(lineQueueCount);
}


std::unique_ptr<Cubemap> Renderer::createCubemapFromHdrTexture(const HdrTexture *hdrTexture, const glm::ivec2 &size) const
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

std::unique_ptr<Cubemap> Renderer::generateIrradianceMap(const Cubemap *cubemap, const glm::ivec2 &size) const
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

std::unique_ptr<Cubemap> Renderer::generatePreFilterMap(const Cubemap *cubemap, const glm::ivec2 &size) const
{
    constexpr int maxMipLevels = 5;
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

void Renderer::drawFullscreenTriangleNow() const
{
    glBindVertexArray(mFullscreenTriangle.vao());
    glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
}

const TextureBufferObject &Renderer::getPrimaryBuffer() const
{
    return *mPrimaryImageBuffer;
}

const TextureBufferObject &Renderer::getAlbedoBuffer() const
{
    return *mAlbedoTextureBuffer;
}

const TextureBufferObject &Renderer::getNormalBuffer() const
{
    return *mNormalTextureBuffer;
}

const TextureBufferObject &Renderer::getPositionBuffer() const
{
    return *mPositionTextureBuffer;
}

const TextureBufferObject &Renderer::getEmissiveBuffer() const
{
    return *mEmissiveTextureBuffer;
}

const TextureBufferObject &Renderer::getLightBuffer() const
{
    return *mLightTextureBuffer;
}

const TextureBufferObject &Renderer::getDepthBuffer() const
{
    return *mDepthTextureBuffer;
}

const TextureBufferObject &Renderer::getRoughnessBuffer() const
{
    return *mRoughnessTextureBuffer;
}

const TextureBufferObject &Renderer::getMetallicBuffer() const
{
    return *mMetallicTextureBuffer;
}

const TextureBufferObject &Renderer::getDeferredLightingBuffer() const
{
    return *mDeferredLightingTextureBuffer;
}

std::vector<graphics::DirectionalLight> &Renderer::getDirectionalLights()
{
    return mDirectionalLightQueue;
}

const TextureBufferObject &Renderer::getSsrBuffer() const
{
    return *mSsrDataTextureBuffer;
}

const TextureBufferObject& Renderer::getReflectionBuffer() const
{
    return *mReflectionTextureBuffer;
}

const TextureBufferObject& Renderer::getDebugBuffer() const
{
    return *mDebugTextureBuffer;
}

float Renderer::getCurrentEV100() const
{
    return mCurrentEV100;
}

float Renderer::getCurrentExposure() const
{
    const float maxLuminance = 1.2f * glm::pow(2.f, getCurrentEV100());
    return 1.f / maxLuminance;
}

void Renderer::setIblMultiplier(const float multiplier)
{
    mIblLuminanceMultiplier = glm::abs(multiplier);
}

void Renderer::rendererGuiNewFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

