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
#include "ThreadGroupSizes.h"
#include "shader/ShaderCompilation.h"

Renderer::Renderer() :
    mCurrentRenderBufferSize(window::bufferSize()),
    mFullscreenTriangle(primitives::fullscreenTriangle()),
    mUnitSphere(primitives::invertedSphere()),
    mLine(primitives::line()),
    mGBufferStorage(sizeof(uint32_t) * 0, "GBuffer Storage Block")
{
    // Blending texture data / enabling lerping.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    graphics::pushDebugGroup("Setup");

    constexpr int lutSize = 64;
    mBrdfLutTextureBuffer = generateBrdfLut(glm::ivec2(lutSize));
    mDirectionalAlbedoF1TextureBuffer = generateDirectionalAlbedoF1(glm::ivec2(lutSize));
    mBrdfAverageLutTextureBuffer = generateBrdfAverageLut(lutSize);
    mSpecularMissingTextureBuffer = generateSpecularMissingLut(glm::ivec2(lutSize));
    generateSkybox((file::texturePath() / "hdr/newport/NewportLoft.hdr").string(), glm::ivec2(512));

    initFrameBuffers();
    initTextureRenderBuffers();
    bindBuffers();
    
    setViewportSize();
    graphics::popDebugGroup();
}

void Renderer::initFrameBuffers()
{
    // One, Zero (override any geometry that is further away from the camera).
    mGeometryFramebuffer = std::make_unique<FramebufferObject>(GL_ONE, GL_ZERO, GL_LESS);

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
    mGBufferTexture = std::make_unique<TextureArrayObject>(window::bufferSize(), 3, GL_RGBA32UI, graphics::filter::Nearest, graphics::wrap::ClampToEdge);
    mDepthTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_DEPTH_COMPONENT32F,    graphics::filter::Nearest, graphics::wrap::ClampToBorder);

    mLightTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F, graphics::filter::Nearest, graphics::wrap::ClampToEdge);
    mCombinedLightingTextureBuffer   = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F,  graphics::filter::LinearMipmapLinear, graphics::wrap::ClampToEdge, 8);
    mPrimaryImageBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F,  GL_NEAREST, GL_NEAREST);
    mAuxiliaryImageBuffer            = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F,  GL_NEAREST, GL_NEAREST);
    mSsrDataTextureBuffer            = std::make_unique<TextureBufferObject>(ssrSize,              GL_RGBA16F, graphics::filter::Nearest, graphics::wrap::ClampToEdge);
    mReflectionTextureBuffer         = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F, graphics::filter::Linear,  graphics::wrap::ClampToEdge);
    mDebugTextureBuffer              = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16,  graphics::filter::Linear,  graphics::wrap::ClampToEdge);

    mDebugGeometryTextureBuffer      = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F,  graphics::filter::Linear,  graphics::wrap::ClampToEdge);
    mDebugWhiteFurnaceTextureBuffer  = std::make_unique<TextureBufferObject>(window::bufferSize(), GL_RGBA16F,  graphics::filter::Nearest,  graphics::wrap::ClampToEdge);

    // Make sure that the framebuffers have been set up before calling this function.
    mGeometryFramebuffer->attach(mGBufferTexture.get(), 0, 0);
    mGeometryFramebuffer->attach(mGBufferTexture.get(), 1, 1);
    mGeometryFramebuffer->attach(mGBufferTexture.get(), 2, 2);

    mGeometryFramebuffer->attachDepthBuffer(mDepthTextureBuffer.get());

    // Reflection Buffer
    mSsrFramebuffer->attach(mSsrDataTextureBuffer.get(), 0);

    mReflectionFramebuffer->attach(mReflectionTextureBuffer.get(), 0);
    
    // Deferred Lighting.
    mDeferredLightFramebuffer->attach(mCombinedLightingTextureBuffer.get(), 0);

    // Debug.
    mDebugFramebuffer->attach(mDebugTextureBuffer.get(), 0);
}

void Renderer::bindBuffers()
{
    int bindPoint = 0;
    mCamera.bindToSlot(++bindPoint);
    mDirectionalLightBlock.bindToSlot(++bindPoint);
    mPointLightBlock.bindToSlot(++bindPoint);
    mSpotlightBlock.bindToSlot(++bindPoint);
    mSsrBlock.bindToSlot(++bindPoint);
    mDebugGBufferBlock.bindToSlot(++bindPoint);

    mDirectionalLightShader.block("CameraBlock", mCamera.getBindPoint());
    mDirectionalLightShader.block("DirectionalLightBlock", mDirectionalLightBlock.getBindPoint());

    mPointLightShader.block("CameraBlock", mCamera.getBindPoint());
    mPointLightShader.block("PointLightBlock", mPointLightBlock.getBindPoint());

    mSpotlightShader.block("CameraBlock", mCamera.getBindPoint());
    mSpotlightShader.block("SpotlightBlock", mSpotlightBlock.getBindPoint());

    mScreenSpaceReflectionsShader.block("CameraBlock", mCamera.getBindPoint());
    mScreenSpaceReflectionsShader.block("ScreenSpaceReflectionsBlock", mSsrBlock.getBindPoint());

    mColourResolveShader.block("CameraBlock", mCamera.getBindPoint());
    mColourResolveShader.block("ScreenSpaceReflectionsBlock", mSsrBlock.getBindPoint());

    mDebugGBufferShader.block("DebugGBufferBlock", mDebugGBufferBlock.getBindPoint());

    mIblShader.block("CameraBlock", mCamera.getBindPoint());
    mWhiteFurnaceTestShader.block("CameraBlock", mCamera.getBindPoint());
}

void Renderer::detachTextureRenderBuffersFromFrameBuffers() const
{
    mGeometryFramebuffer->detach(0);
    mGeometryFramebuffer->detach(1);
    mGeometryFramebuffer->detach(2);
    mGeometryFramebuffer->detachDepthBuffer();
    
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
        // mGeometryFramebuffer->clear(camera.clearColour);
        mGeometryFramebuffer->clear(0, camera.clearColour);
        mGeometryFramebuffer->clear(1, camera.clearColour);
        mGeometryFramebuffer->clear(2, camera.clearColour);
        // mGeometryFramebuffer->clear(3, camera.clearColour);
        // mGeometryFramebuffer->clear(4, camera.clearColour);
        // mGeometryFramebuffer->clear(5, camera.clearColour);
        mGeometryFramebuffer->clearDepthBuffer();
        mGBufferTexture->clear();

        const glm::mat4 cameraProjectionMatrix = glm::perspective(camera.fovY, window::aspectRatio(), camera.nearClipDistance, camera.farClipDistance);
        const glm::mat4 vpMatrix = cameraProjectionMatrix * camera.viewMatrix;

        mCamera->viewMatrix = camera.viewMatrix;
        mCamera->inverseVpMatrix = glm::inverse(vpMatrix);
        mCamera->position = glm::vec3(glm::inverse(camera.viewMatrix) * glm::vec4(glm::vec3(0.f), 1.f));
        mCamera->exposure = exposure;
        mCamera->zNear = camera.nearClipDistance;
        mCamera->zFar = camera.farClipDistance;
        mCamera.updateGlsl();

        for (const auto &rqo : mRenderQueue)
        {
            if (rqo.shader.expired())
                continue;

            const auto shader = rqo.shader.lock();
            shader->bind();
            shader->set("u_mvp_matrix", vpMatrix * rqo.matrix);
            shader->set("u_model_matrix", rqo.matrix);
            shader->block("CameraBlock", mCamera.getBindPoint());
            // shader->image("storageGBuffer", mGBufferTexture->getId(), mGBufferTexture->getFormat(), 0, true, GL_WRITE_ONLY);
            // shader->set("storageGBufferSsbo", mGBufferStorage.getBindPoint());
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
        
        mLightTextureBuffer->clear();

        mDirectionalLightShader.bind();

        mDirectionalLightShader.set("depthBufferTexture", mDepthTextureBuffer->getId(), 0);
        mDirectionalLightShader.set("directionalAlbedoWhite", mDirectionalAlbedoF1TextureBuffer->getId(), 3);
        mDirectionalLightShader.set("directionalAlbedoAverageWhite", mBrdfAverageLutTextureBuffer->getId(), 4);
        mDirectionalLightShader.image("storageGBuffer", mGBufferTexture->getId(), mGBufferTexture->getFormat(), 0, true, GL_READ_ONLY);
        mDirectionalLightShader.image("lighting", mLightTextureBuffer->getId(), mLightTextureBuffer->getFormat(), 1, false, GL_READ_WRITE);

        const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(mCurrentRenderBufferSize) / glm::vec2(FULLSCREEN_THREAD_GROUP_SIZE)));

        for (const graphics::DirectionalLight &directionalLight : mDirectionalLightQueue)
        {
            mDirectionalLightBlock->direction = glm::vec4(directionalLight.direction, 0.f);
            mDirectionalLightBlock->intensity = glm::vec4(directionalLight.colourIntensity, 0.f);
            std::copy(directionalLight.vpMatrices.begin(), directionalLight.vpMatrices.end(), std::begin(mDirectionalLightBlock->vpMatrices));
            std::copy(directionalLight.cascadeDepths.begin(), directionalLight.cascadeDepths.end(), std::begin(mDirectionalLightBlock->cascadeDistances));
            mDirectionalLightBlock->bias = directionalLight.shadowBias;
            mDirectionalLightBlock->cascadeCount = static_cast<int>(directionalLight.cascadeDepths.size());

            mDirectionalLightBlock.updateGlsl();
            mDirectionalLightShader.set("u_shadow_map_texture", directionalLight.shadowMap->getId(), 1);

            glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
        }
        
        PROFILE_SCOPE_END(directionalLightTimer);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(pointLightTimer, "Point Lighting");
        graphics::pushDebugGroup("Point Lighting");
        
        mPointLightShader.bind();
        
        mPointLightShader.set("depthBufferTexture", mDepthTextureBuffer->getId(), 0);
        mPointLightShader.set("directionalAlbedoWhite", mDirectionalAlbedoF1TextureBuffer->getId(), 3);
        mPointLightShader.set("directionalAlbedoAverageWhite", mBrdfAverageLutTextureBuffer->getId(), 4);
        mPointLightShader.image("storageGBuffer", mGBufferTexture->getId(), mGBufferTexture->getFormat(), 0, true, GL_READ_ONLY);
        mPointLightShader.image("lighting", mLightTextureBuffer->getId(), mLightTextureBuffer->getFormat(), 1, false, GL_READ_WRITE);

        for (const auto &pointLight : mPointLightQueue)
        {
            mPointLightBlock->position = glm::vec4(pointLight.position, 1.f);
            mPointLightBlock->intensity = glm::vec4(pointLight.colourIntensity, 0.f);
            mPointLightBlock->invSqrRadius = 1.f / (pointLight.radius * pointLight.radius);
            mPointLightBlock->zFar = pointLight.radius;
            mPointLightBlock->softnessRadius = pointLight.softnessRadius;
            mPointLightBlock->bias = pointLight.bias;
            const glm::mat4 pointLightModelMatrix = glm::translate(glm::mat4(1.f), pointLight.position) * glm::scale(glm::mat4(1.f), glm::vec3(pointLight.radius));
            mPointLightBlock->mvpMatrix = vpMatrix * pointLightModelMatrix;
            mPointLightBlock.updateGlsl();

            mPointLightShader.set("u_shadow_map_texture", pointLight.shadowMap->getId(), 1);

            glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
        }
        
        
        PROFILE_SCOPE_END(pointLightTimer);
        graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(spotLightTimer, "Spot Light");
        graphics::pushDebugGroup("Spot Light");
        
        mSpotlightShader.bind();
        
        mSpotlightShader.set("depthBufferTexture", mDepthTextureBuffer->getId(), 0);
        mSpotlightShader.set("directionalAlbedoWhite", mDirectionalAlbedoF1TextureBuffer->getId(), 3);
        mSpotlightShader.set("directionalAlbedoAverageWhite", mBrdfAverageLutTextureBuffer->getId(), 4);
        mPointLightShader.image("storageGBuffer", mGBufferTexture->getId(), mGBufferTexture->getFormat(), 0, true, GL_READ_ONLY);
        mPointLightShader.image("lighting", mLightTextureBuffer->getId(), mLightTextureBuffer->getFormat(), 1, false, GL_READ_WRITE);

        for (const graphics::Spotlight &spotLight : mSpotlightQueue)
        {
            mSpotlightShader.set("u_shadow_map_texture", spotLight.shadowMap->getId(), 1);

            mSpotlightBlock->position = glm::vec4(spotLight.position, 1.f);
            mSpotlightBlock->direction = glm::vec4(spotLight.direction, 0.f);
            mSpotlightBlock->invSqrRadius = 1.f / (spotLight.radius * spotLight.radius);
            mSpotlightBlock->intensity = glm::vec4(spotLight.colourIntensity, 0.f);
            mSpotlightBlock->bias = spotLight.shadowBias;
            mSpotlightBlock->zFar = spotLight.radius;
            mSpotlightBlock->vpMatrix = spotLight.vpMatrix;
            const float lightAngleScale = 1.f / glm::max(0.001f, (spotLight.cosInnerAngle - spotLight.cosOuterAngle));
            const float lightAngleOffset = -spotLight.cosOuterAngle * lightAngleScale;
            mSpotlightBlock->angleScale = lightAngleScale;
            mSpotlightBlock->angleOffset = lightAngleOffset;

            mSpotlightBlock.updateGlsl();

            glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);
        }
        
        PROFILE_SCOPE_END(spotLightTimer);
        graphics::popDebugGroup();

        shadeDistantLightProbe();

        // todo: ibl
        // PROFILE_SCOPE_BEGIN(screenSpace, "Reflections");
        // graphics::pushDebugGroup("Reflections");
        //
        // const glm::mat4 scaleTextureSpace = glm::scale(glm::mat4(1.f), glm::vec3(0.5f, 0.5f, 1.f));
        // const glm::mat4 translateTextureSpace = glm::translate(glm::mat4(1.f), glm::vec3(0.5f, 0.5f, 0.f));
        // const glm::mat4 scaleFrameBufferSpace = glm::scale(glm::mat4(1.f), glm::vec3(mLightTextureBuffer->getSize().x, mLightTextureBuffer->getSize().y, 1));
        //
        // const glm::mat4 viewToPixelCoordMatrix = scaleFrameBufferSpace * translateTextureSpace * scaleTextureSpace * cameraProjectionMatrix;
        // mSsrBlock->projection = viewToPixelCoordMatrix;
        // mSsrBlock->zNear = -camera.nearClipDistance;
        // mSsrBlock->colourMaxLod = static_cast<int>(mDeferredLightingTextureBuffer->getMipLevels());
        // mSsrBlock->luminanceMultiplier = mIblLuminanceMultiplier;
        // mSsrBlock->maxDistanceFalloff = mRoughnessFallOff;
        // mSsrBlock.updateGlsl();
        //
        // // Render at half-resolution for performance.
        // glViewport(0, 0, mSsrDataTextureBuffer->getSize().x, mSsrDataTextureBuffer->getSize().y);
        // mSsrFramebuffer->bind();
        // mSsrFramebuffer->clear(glm::vec4(0.f));
        // mScreenSpaceReflectionsShader->bind();
        //
        // mDepthTextureBuffer->setBorderColour(glm::vec4(0.f, 0.f, 0.f, 1.f));
        //
        // mScreenSpaceReflectionsShader->set("u_positionTexture", mPositionTextureBuffer->getId(), 0);
        // mScreenSpaceReflectionsShader->set("u_normalTexture", mNormalTextureBuffer->getId(), 1);
        // mScreenSpaceReflectionsShader->set("u_depthTexture", mDepthTextureBuffer->getId(), 2);
        // mScreenSpaceReflectionsShader->set("u_roughnessTexture", mRoughnessTextureBuffer->getId(), 3);
        //
        // drawFullscreenTriangleNow();
        //
        // glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
        //
        // mReflectionFramebuffer->bind();
        // mReflectionFramebuffer->clear(glm::vec4(0.f));
        // mColourResolveShader->bind();
        //
        // mColourResolveShader->set("u_albedoTexture",             mAlbedoTextureBuffer->getId(),     0);
        // mColourResolveShader->set("u_positionTexture",           mPositionTextureBuffer->getId(),   1);
        // mColourResolveShader->set("u_normalTexture",             mNormalTextureBuffer->getId(),     2);
        // mColourResolveShader->set("u_roughnessTexture",          mRoughnessTextureBuffer->getId(),  3);
        // mColourResolveShader->set("u_metallicTexture",           mMetallicTextureBuffer->getId(),   4);
        // mColourResolveShader->set("u_reflectionDataTexture",     mSsrDataTextureBuffer->getId(),    5);
        // mColourResolveShader->set("u_colourTexture",             mDeferredLightingTextureBuffer->getId(), 6);
        // mColourResolveShader->set("u_depthTexture",              mDepthTextureBuffer->getId(),      7);
        // mColourResolveShader->set("u_irradianceTexture",         mIrradianceMap->getId(),           8);
        // mColourResolveShader->set("u_pre_filterTexture",         mPreFilterMap->getId(),            9);
        // mColourResolveShader->set("u_brdfLutTexture",            mBrdfLutTextureBuffer->getId(),    10);
        // mColourResolveShader->set("u_emissiveTexture", mEmissiveTextureBuffer->getId(), 11);
        //
        // drawFullscreenTriangleNow();
        //
        // glViewport(0, 0, window::bufferSize().x, window::bufferSize().y);
        //
        // PROFILE_SCOPE_END(screenSpace);
        // graphics::popDebugGroup();
        PROFILE_SCOPE_BEGIN(deferredTimer, "Skybox Pass");
        graphics::pushDebugGroup("Combine + Skybox Pass");
        
        // Deferred Lighting step.
        
        // mDeferredLightFramebuffer->bind();
        // mDeferredLightFramebuffer->clear(glm::vec4(glm::vec3(0.f), 0.f));
        mCombinedLightingTextureBuffer->clear();
        
        mCombineLightingShader.bind();
        
        const glm::mat4 viewMatrixNoPosition = glm::mat4(glm::mat3(camera.viewMatrix));
        const glm::mat4 inverseViewProjection = glm::inverse(cameraProjectionMatrix * viewMatrixNoPosition);

        mCombineLightingShader.set("u_irradiance_texture", mLightTextureBuffer->getId(), 0);
        // todo: emissive buffer should go straight in irradiance texture.
        // mDeferredLightShader.set("u_emissive_texture", mEmissiveTextureBuffer->getId(), 1);
        mCombineLightingShader.set("depthBufferTexture", mDepthTextureBuffer->getId(), 2);
        mCombineLightingShader.set("u_skybox_texture", mHdrSkybox->getId(), 3);
        mCombineLightingShader.set("u_reflection_texture", mReflectionTextureBuffer->getId(), 4);

        mCombineLightingShader.set("u_inverse_vp_matrix", inverseViewProjection);
        mCombineLightingShader.set("u_luminance_multiplier", mIblLuminanceMultiplier);
        mCombineLightingShader.set("u_exposure", exposure);
        mCombineLightingShader.image("lighting", mCombinedLightingTextureBuffer->getId(), mCombinedLightingTextureBuffer->getFormat(), 1, false, GL_READ_WRITE);

        glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);

        blurTexture(*mCombinedLightingTextureBuffer);

        PROFILE_SCOPE_END(deferredTimer);
        graphics::popDebugGroup();
        
        PROFILE_SCOPE_BEGIN(postProcessTimer, "Post-processing Stack");
        graphics::pushDebugGroup("Post-processing Pass");
        
        graphics::copyTexture2D(*mCombinedLightingTextureBuffer, *mPrimaryImageBuffer);
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
        mDebugShader.bind();

        for (const auto & [vao, count, modelMatrix, colour] : mDebugQueue)
        {
            mDebugShader.set("u_mvp_matrix", vpMatrix * modelMatrix);
            mDebugShader.set("u_colour", colour);
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        }

        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        mLineShader.bind();
        mLineShader.set("u_mvp_matrix", vpMatrix);
        glBindVertexArray(mLine.vao());
        for (const auto &[startPosition, endPosition, colour] : mLineQueue)
        {
            mLineShader.set("u_locationA", startPosition);
            mLineShader.set("u_locationB", endPosition);
            mLineShader.set("u_colour",    colour);
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
    mDirectionalLightShadowShader.bind();
    
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
                mDirectionalLightShadowShader.set("u_mvp_matrix", mvp);
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
    mPointLightShadowShader.bind();
    
    for (auto &pointLight : mPointLightQueue)
    {
        PROFILE_SCOPE_BEGIN(pointLightTimer, "Point Light Shadow Pass");
        graphics::pushDebugGroup("Point Light Pass");
        const glm::ivec2 size = pointLight.shadowMap->getSize();
        glViewport(0, 0, size.x, size.y);
        
        mPointLightShadowShader.set("u_light_pos", pointLight.position);
        mPointLightShadowShader.set("u_z_far", pointLight.radius);
        
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
                mPointLightShadowShader.set("u_model_matrix", modelMatrix);
                mPointLightShadowShader.set("u_mvp_matrix", mvp);
                
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
    
    mSpotlightShadowShader.bind();
    mShadowFramebuffer->bind();
    
    for (const graphics::Spotlight &spotlight : mSpotlightQueue)
    {
        const glm::ivec2 size = spotlight.shadowMap->getSize();
        glViewport(0, 0, size.x, size.y);
        
        mShadowFramebuffer->attachDepthBuffer(spotlight.shadowMap.get());
        mShadowFramebuffer->clearDepthBuffer();
        
        const glm::mat4 &vpMatrix = spotlight.vpMatrix;
        mSpotlightShadowShader.set("u_light_pos", spotlight.position);
        mSpotlightShadowShader.set("u_z_far", spotlight.radius);
        
        for (const auto &rqo : mRenderQueue)
        {
            const glm::mat4 mvpMatrix = vpMatrix * rqo.matrix;
            mSpotlightShadowShader.set("u_mvp_matrix", mvpMatrix);
            mSpotlightShadowShader.set("u_model_matrix", rqo.matrix);
            
            glBindVertexArray(rqo.vao);
            glDrawElements(rqo.drawMode, rqo.indicesCount, GL_UNSIGNED_INT, nullptr);
        }
        
        mShadowFramebuffer->detachDepthBuffer();
    }
    
    graphics::popDebugGroup();
}

void Renderer::shadeDistantLightProbe()
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Distant Light Probe");

    mIblShader.image("storageGBuffer", mGBufferTexture->getId(), mGBufferTexture->getFormat(), 0, true, GL_READ_ONLY);
    mIblShader.image("lighting", mLightTextureBuffer->getId(), mLightTextureBuffer->getFormat(), 1, false, GL_READ_WRITE);
    mIblShader.set("depthBufferTexture", mDepthTextureBuffer->getId(), 0);
    mIblShader.set("u_brdf_lut_texture", mBrdfLutTextureBuffer->getId(), 1);
    mIblShader.set("directionAlbedoF1AverageTexture", mBrdfAverageLutTextureBuffer->getId(), 2);
    mIblShader.set("missingSpecularLutTexture", mSpecularMissingTextureBuffer->getId(), 3);
    mIblShader.set("u_irradiance_texture", mIrradianceMap->getId(), 4);
    mIblShader.set("u_pre_filter_texture", mPreFilterMap->getId(), 5);
    mIblShader.set("u_luminance_multiplier", mIblLuminanceMultiplier);

    mIblShader.bind();
    const auto threadGroupSize = glm::ivec2(ceil(glm::vec2(mLightTextureBuffer->getSize()) / glm::vec2(FULLSCREEN_THREAD_GROUP_SIZE)));
    glDispatchCompute(threadGroupSize.x, threadGroupSize.y, 1);

    graphics::popDebugGroup();
}

void Renderer::blurTexture(const TextureBufferObject& texture)
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Auxilliary Blur Pass");
    const uint32_t mipLevels = texture.getMipLevels();
    const glm::ivec2 &size = texture.getSize();
    mBlurFramebuffer->bind();
    mBlurShader.bind();
    mBlurShader.set("u_texture", texture.getId(), 0);

    for (int i = 1; i < mipLevels; ++i)
    {
        glViewport(0, 0, size.x >> i, size.y >> i);
        mBlurShader.set("u_mip_level", i - 1);
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


std::unique_ptr<Cubemap> Renderer::createCubemapFromHdrTexture(const HdrTexture *hdrTexture, const glm::ivec2 &size)
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
    mHdrToCubemapShader.bind();
    mHdrToCubemapShader.set("u_texture", hdrTexture->getId(), 0);
    
    glViewport(0, 0, size.x, size.y);
    
    glBindVertexArray(mFullscreenTriangle.vao());
    
    for (int i = 0; i < 6; ++i)
    {
        mHdrToCubemapShader.set("u_view_matrix", views[i]);
        auxiliaryFrameBuffer.attach(cubemap.get(), 0, i);
        auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));
        
        glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
        
        auxiliaryFrameBuffer.detach(0);
    }
    
    return cubemap;
}

std::unique_ptr<Cubemap> Renderer::generateIrradianceMap(const Cubemap *cubemap, const glm::ivec2 &size)
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
    mCubemapToIrradianceShader.bind();
    mCubemapToIrradianceShader.set("u_environment_texture", cubemap->getId(), 0);
    
    glViewport(0, 0, size.x, size.y);
    
    glBindVertexArray(mFullscreenTriangle.vao());
    
    for (int i = 0; i < 6; ++i)
    {
        mCubemapToIrradianceShader.set("u_view_matrix", views[i]);
        auxiliaryFrameBuffer.attach(irradiance.get(), 0, i);
        auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));
        
        glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
        
        auxiliaryFrameBuffer.detach(0);
    }
    
    return irradiance;
}

std::unique_ptr<Cubemap> Renderer::generatePreFilterMap(const Cubemap *cubemap, const glm::ivec2 &size)
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
    mPreFilterShader.bind();
    mPreFilterShader.set("u_environment_texture", cubemap->getId(), 0);
    
    glBindVertexArray(mFullscreenTriangle.vao());
    
    for (int mip = 0; mip < maxMipLevels; ++mip)
    {
        const glm::ivec2 mipSize = size >> mip;
        glViewport(0, 0, mipSize.x, mipSize.y);
        
        const float roughness = static_cast<float>(mip) / (static_cast<float>(maxMipLevels) - 1.f);
        mPreFilterShader.set("u_roughness", roughness);
        
        for (int i = 0; i < 6; ++i)
        {
            mPreFilterShader.set("u_view_matrix", views[i]);
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
    auto lut = std::make_unique<TextureBufferObject>(size, GL_RGBA16F, graphics::filter::Linear, graphics::wrap::ClampToEdge);

    mIntegrateBrdfShader.bind();
    mIntegrateBrdfShader.image("lut", lut->getId(), lut->getFormat(), 5, false);
    const glm::uvec2 groupSize = glm::ceil(static_cast<glm::vec2>(size / 8));
    glDispatchCompute(groupSize.x, groupSize.y, 1);

    return lut;
}

std::unique_ptr<TextureBufferObject> Renderer::generateSpecularMissingLut(const glm::ivec2& size)
{
    auto lut = std::make_unique<TextureBufferObject>(size, GL_R16F, graphics::filter::Linear, graphics::wrap::ClampToEdge);

    mIntergateSpecularMissing.bind();
    mIntergateSpecularMissing.set("directionalAlbedoWhite", mDirectionalAlbedoF1TextureBuffer->getId(), 0);
    mIntergateSpecularMissing.set("directionalAlbedoAverageWhite", mBrdfAverageLutTextureBuffer->getId(), 1);
    mIntergateSpecularMissing.image("specMissing", lut->getId(), lut->getFormat(), 0, false);
    const glm::uvec2 groupSize = glm::ceil(static_cast<glm::vec2>(size / 8));
    glDispatchCompute(groupSize.x, groupSize.y, 1);

    return lut;
}

std::unique_ptr<TextureBufferObject> Renderer::generateBrdfAverageLut(const uint32_t size)
{
    auto lut = std::make_unique<TextureBufferObject>(glm::ivec2(size, 1), GL_RG16F, graphics::filter::Linear, graphics::wrap::ClampToEdge);
    lut->setDebugName("BRDF Average LUT");

    mIntegrateBrdfAverageShader.bind();
    mIntegrateBrdfAverageShader.set("brdfLut", mDirectionalAlbedoF1TextureBuffer->getId(), 0);
    mIntegrateBrdfAverageShader.image("brdfAverageLut", lut->getId(), lut->getFormat(), 0, false, GL_WRITE_ONLY);

    const uint32_t groupSize = glm::ceil(size / 8);
    glDispatchCompute(groupSize, 1, 1);

    return lut;
}

std::unique_ptr<TextureBufferObject> Renderer::generateDirectionalAlbedoF1(const glm::ivec2& size)
{
    auto lut = std::make_unique<TextureBufferObject>(size, GL_R16F, graphics::filter::Linear, graphics::wrap::ClampToEdge);
    lut->setDebugName("Directional Albedo F1");

    mIntegrateDirectionalAlbedoF1.bind();
    mIntegrateDirectionalAlbedoF1.image("lut", lut->getId(), lut->getFormat(), 0, false, GL_WRITE_ONLY);
    const glm::uvec2 groupSize = glm::ceil(static_cast<glm::vec2>(size / 8));
    glDispatchCompute(groupSize.x, groupSize.y, 1);

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

const TextureBufferObject &Renderer::getLightBuffer() const
{
    return *mLightTextureBuffer;
}

const TextureBufferObject &Renderer::getDepthBuffer() const
{
    return *mDepthTextureBuffer;
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

const TextureBufferObject& Renderer::getFromGBuffer(const graphics::gbuffer type, const bool gammaCorrect, const glm::vec4 &defaultValue)
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("GBuffer Query");
    mDebugGBufferBlock->id = toInt(type);
    mDebugGBufferBlock->gammaCorrect = static_cast<int>(gammaCorrect);
    mDebugGBufferBlock->defaultValue = defaultValue;
    mDebugGBufferBlock.updateGlsl();

    mDebugGBufferShader.bind();
    mDebugGBufferShader.image("storageGBuffer", mGBufferTexture->getId(), mGBufferTexture->getFormat(), 0, true, GL_READ_ONLY);
    mDebugGBufferShader.image("debug", mDebugGeometryTextureBuffer->getId(), mDebugGeometryTextureBuffer->getFormat(), 1, false, GL_WRITE_ONLY);

    const glm::vec2 screenSize = mDebugGeometryTextureBuffer->getSize();
    const glm::ivec2 numThreadGroups = glm::ceil(screenSize / glm::vec2(DEBUG_GBUFFER_THREAD_SIZE));
    glDispatchCompute(numThreadGroups.x, numThreadGroups.y, 1);

    graphics::popDebugGroup();
    return *mDebugGeometryTextureBuffer;
}

const TextureBufferObject &Renderer::whiteFurnaceTest()
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("White Furnace Test");

    mWhiteFurnaceTestShader.bind();
    mWhiteFurnaceTestShader.image("storageGBuffer", mGBufferTexture->getId(), mGBufferTexture->getFormat(), 0, true, GL_READ_ONLY);
    mWhiteFurnaceTestShader.image("lighting", mDebugWhiteFurnaceTextureBuffer->getId(), mDebugWhiteFurnaceTextureBuffer->getFormat(), 1, false, GL_READ_WRITE);
    mWhiteFurnaceTestShader.set("depthBufferTexture", mDepthTextureBuffer->getId(), 0);
    mWhiteFurnaceTestShader.set("u_brdf_lut_texture", mBrdfLutTextureBuffer->getId(), 1);
    mWhiteFurnaceTestShader.set("directionAlbedoF1AverageTexture", mBrdfAverageLutTextureBuffer->getId(), 2);
    mWhiteFurnaceTestShader.set("missingSpecularLutTexture", mSpecularMissingTextureBuffer->getId(), 3);

    const glm::vec2 screenSize = mDebugWhiteFurnaceTextureBuffer->getSize();
    const glm::ivec2 numThreadGroups = glm::ceil(screenSize / glm::vec2(FULLSCREEN_THREAD_GROUP_SIZE));
    glDispatchCompute(numThreadGroups.x, numThreadGroups.y, 1);

    graphics::popDebugGroup();
    return *mDebugWhiteFurnaceTextureBuffer;
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

