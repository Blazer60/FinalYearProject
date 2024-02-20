/**
 * @file Renderer.h
 * @author Ryan Purse
 * @date 14/07/2023
 */


#pragma once

#include "Buffers.h"
#include "CameraSettings.h"
#include "GraphicsDefinitions.h"
#include "GraphicsLighting.h"
#include "Materials.h"
#include "Mesh.h"
#include "Shader.h"
#include "UniformBufferObject.h"
#include "WindowHelpers.h"
#include "CameraBlock.h"
#include "DebugGBufferBlock.h"
#include "DirectionalLightBlock.h"
#include "PointLightBlock.h"
#include "ScreenSpaceReflectionsBlock.h"
#include "SpotlightBlock.h"


/**
 * @author Ryan Purse
 * @date 14/07/2023
 */
class Renderer
{
public:
    Renderer();
    Renderer(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    Renderer operator=(const Renderer&) = delete;
    Renderer operator=(Renderer&&) = delete;
    ~Renderer() = default;
    
    /**
     * @brief Draws an element to the geometry buffer.
     * @param vao - Vertex Array Object.
     * @param indicesCount - The number of indices that make up the geometry.
     * @param shader - The shader used to drawUi the element to the geometry buffer.
     * @param renderMode - What primitive to use when rendering.
     * @param matrix - The model matrix for this object (used for shadow mapping).
     * @param onDraw - An event callback that is called just before rendering.
     */
    void drawMesh(
        uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader,
        graphics::drawMode renderMode, const glm::mat4 &matrix,
        const graphics::DrawCallback &onDraw);
    
    /**
     * @brief Draws an element to the geometry buffer.
     * @param subMesh - The mesh that you want to be drawn.
     * @param material - The material (shader) used to drawUi the mesh.
     * @param matrix - The model's matrix (used for shadow mapping).
     */
    void drawMesh(const SubMesh &subMesh, Material &material, const glm::mat4 &matrix);
    
    /**
     * @brief Draws a number of meshes to the geometry buffer with the same model matrix.
     * @param mesh - N sub-meshes that you want to be drawn to the geometry buffer.
     * @param materials - N materials used to drawUi each mesh. If the material count is one, then only that material
     * will be used.
     * @param matrix - the model's matrix (used for shadow mapping).
     */
    void drawMesh(const SharedMesh &mesh, const SharedMaterials &materials, const glm::mat4 &matrix);

    /**
     * @brief Draws an element to the debug buffer.
     * @param vao - Vertex Array Object.
     * @param indicesCount - The number of indices that make up the geometry.
     * @param matrix - The model matrix for this object.
     * @param colour - The colour the mesh should be.
     */
    void drawDebugMesh(uint32_t vao, int32_t indicesCount, const glm::mat4 &matrix, const glm::vec3 &colour);

    /**
     * @brief Draws an element to the debug buffer.
     * @param subMesh - The mesh that you want to be drawn.
     * @param matrix - The model's matrix.
     * @param colour - The colour the mesh should be.
     */
    void drawDebugMesh(const SubMesh &subMesh, const glm::mat4 &matrix, const glm::vec3 &colour);

    /**
     * @brief Draws a number of meshes to the debug buffer with the same model matrix.
     * @param mesh - N sub-meshes that you want to be drawn to the debug buffer.
     * @param matrix - The model's matrix.
     * @param colour - The colour the mesh should be.
     */
    void drawDebugMesh(const SharedMesh &mesh, const glm::mat4 &matrix, const glm::vec3 &colour);

    /**
     * @brief Draws a line in world space to the debug buffer.
     * @param startPosition - The start position of the line.
     * @param endPosition - The end position of the line.
     * @param colour - The colour that the line should be.
     */
    void drawDebugLine(const glm::vec3 &startPosition, const glm::vec3 &endPosition, const glm::vec3 &colour);
    
    /**
     * @brief Submits a camera that will be used for the render pipeline.
     */
    void submit(const CameraSettings &cameraSettings);
    
    /**
     * @brief Submits a directional light that will be used to light the world and cast shadows.
     */
    void submit(const graphics::DirectionalLight &directionalLight);
    
    /**
     * @brief Submits a point light that will be used to light the world.
     */
    void submit(const graphics::PointLight &pointLight);
    
    /**
     * @brief Submits a spot light that will be used to light the world.
     */
    void submit(const graphics::Spotlight &spotLight);
    
    /**
     * Starts rendering everything that was submitted to the renderer this frame.
     */
    void render();
    
    /**
     * @brief Resets the data fro the next round of rendering. This is split so that ImGui can display information
     * before being reset.
     */
    void clear();
    
    /**
     * @returns false if debug message was failed to be setup. This is most likely due to the openGl version being
     * less than 4.3.
     * @see https://docs.gl/gl4/glDebugMessageCallback
     */
    static bool debugMessageCallback(GLDEBUGPROC callback);

    /**
     * @brief Gets the current version of OpenGL that is being used.
     */
    [[nodiscard]] static std::string getVersion();
    
    void generateSkybox(std::string_view path, glm::ivec2 desiredSize);
    
    /**
     * @brief Draws a triangle to the screen so that fullscreen passes can be performs without
     * having to worry about mesh data. Make sure that a shader and fbo are already bound before
     * calling this method.
     */
    void drawFullscreenTriangleNow() const;
    
    /**
     * @brief Resets some internal state so that the GUI system doesn't override anything in the renderer.
     */
    static void rendererGuiNewFrame();
    
    [[nodiscard]] const TextureBufferObject &getPrimaryBuffer() const;
    [[nodiscard]] const TextureBufferObject &getDeferredLightingBuffer() const;
    [[nodiscard]] const TextureBufferObject &getAlbedoBuffer() const;
    [[nodiscard]] const TextureBufferObject &getNormalBuffer() const;
    [[nodiscard]] const TextureBufferObject &getPositionBuffer() const;
    [[nodiscard]] const TextureBufferObject &getEmissiveBuffer() const;
    [[nodiscard]] const TextureBufferObject &getLightBuffer() const;
    [[nodiscard]] const TextureBufferObject &getDepthBuffer() const;
    [[nodiscard]] const TextureBufferObject &getRoughnessBuffer() const;
    [[nodiscard]] const TextureBufferObject &getMetallicBuffer() const;
    [[nodiscard]] const TextureBufferObject &getSsrBuffer() const;
    [[nodiscard]] const TextureBufferObject &getReflectionBuffer() const;
    [[nodiscard]] const TextureBufferObject &getDebugBuffer() const;
    [[nodiscard]] const TextureBufferObject &getFromGBuffer(graphics::gbuffer type, bool gammaCorrect, const glm::vec4 &defaultValue=glm::vec4(0.f, 0.f, 0.f, 1.f));
    
    [[nodiscard]] std::vector<graphics::DirectionalLight> &getDirectionalLights();
    
    [[nodiscard]] float getCurrentEV100() const;
    [[nodiscard]] float getCurrentExposure() const;
    
    void setIblMultiplier(float multiplier);

protected:
    void initFrameBuffers();
    void initTextureRenderBuffers();
    void bindUbos();
    void detachTextureRenderBuffersFromFrameBuffers() const;
    std::unique_ptr<Cubemap> createCubemapFromHdrTexture(const HdrTexture *hdrTexture, const glm::ivec2 &size) const;
    std::unique_ptr<Cubemap> generateIrradianceMap(const Cubemap *cubemap, const glm::ivec2 &size) const;
    std::unique_ptr<Cubemap> generatePreFilterMap(const Cubemap *cubemap, const glm::ivec2 &size) const;
    std::unique_ptr<TextureBufferObject> generateBrdfLut(const glm::ivec2 &size);
    void directionalLightShadowMapping(const CameraSettings &cameraSettings);
    void pointLightShadowMapping() const;
    void spotlightShadowMapping() const;
    void blurTexture(const TextureBufferObject &texture) const;

    static void setViewportSize(const glm::ivec2 &size=window::bufferSize());
    
    std::vector<graphics::RenderQueueObject>        mRenderQueue;
    std::vector<CameraSettings>                     mCameraQueue;
    std::vector<graphics::DirectionalLight>         mDirectionalLightQueue;
    std::vector<graphics::PointLight>               mPointLightQueue;
    std::vector<graphics::Spotlight>                mSpotlightQueue;
    std::vector<graphics::DebugQueueObject>         mDebugQueue;
    std::vector<graphics::LineQueueObject>          mLineQueue;
    
    std::unique_ptr<HdrTexture>  mHdrImage;
    std::unique_ptr<Cubemap>     mHdrSkybox;
    std::unique_ptr<Cubemap>     mIrradianceMap;
    std::unique_ptr<Cubemap>     mPreFilterMap;
    
    std::unique_ptr<FramebufferObject> mDeferredLightFramebuffer;
    std::unique_ptr<FramebufferObject> mGeometryFramebuffer;
    std::unique_ptr<FramebufferObject> mLightFramebuffer;
    std::unique_ptr<FramebufferObject> mShadowFramebuffer;
    std::unique_ptr<FramebufferObject> mSsrFramebuffer;
    std::unique_ptr<FramebufferObject> mReflectionFramebuffer;
    std::unique_ptr<FramebufferObject> mBlurFramebuffer;
    std::unique_ptr<FramebufferObject> mDebugFramebuffer;

    std::unique_ptr<Shader> mDeferredLightShader;
    Shader mDirectionalLightShader;
    std::unique_ptr<Shader> mPointLightShader;
    std::unique_ptr<Shader> mSpotlightShader;
    std::unique_ptr<Shader> mIblShader;
    std::unique_ptr<Shader> mDirectionalLightShadowShader;
    std::unique_ptr<Shader> mPointLightShadowShader;
    std::unique_ptr<Shader> mSpotlightShadowShader;
    std::unique_ptr<Shader> mHdrToCubemapShader;
    std::unique_ptr<Shader> mCubemapToIrradianceShader;
    std::unique_ptr<Shader> mPreFilterShader;
    Shader mIntegrateBrdfShader;
    std::unique_ptr<Shader> mScreenSpaceReflectionsShader;
    std::unique_ptr<Shader> mColourResolveShader;
    std::unique_ptr<Shader> mBlurShader;
    std::unique_ptr<Shader> mDebugShader;
    std::unique_ptr<Shader> mLineShader;
    Shader mDebugGBufferShader;
    
    SubMesh mFullscreenTriangle;
    SubMesh mUnitSphere;
    SubMesh mLine;

    std::unique_ptr<TextureArrayObject> mGBufferTexture;
    std::unique_ptr<TextureBufferObject> mDepthTextureBuffer;
    std::unique_ptr<TextureBufferObject> mLightTextureBuffer;
    std::unique_ptr<TextureBufferObject> mDeferredLightingTextureBuffer;
    std::unique_ptr<TextureBufferObject> mBrdfLutTextureBuffer;
    std::unique_ptr<TextureBufferObject> mPrimaryImageBuffer;
    std::unique_ptr<TextureBufferObject> mAuxiliaryImageBuffer;
    std::unique_ptr<TextureBufferObject> mSsrDataTextureBuffer;
    std::unique_ptr<TextureBufferObject> mReflectionTextureBuffer;
    std::unique_ptr<TextureBufferObject> mDebugTextureBuffer;
    std::unique_ptr<TextureBufferObject> mDebugGeometryTextureBuffer;

    graphics::UniformBufferObject<CameraBlock> mCamera;
    graphics::UniformBufferObject<DirectionalLightBlock> mDirectionalLightBlock;
    graphics::UniformBufferObject<PointLightBlock> mPointLightBlock;
    graphics::UniformBufferObject<SpotlightBlock> mSpotlightBlock;
    graphics::UniformBufferObject<ScreenSpaceReflectionsBlock> mSsrBlock;
    graphics::UniformBufferObject<DebugGBufferBlock> mDebugGBufferBlock;

    glm::ivec2 mCurrentRenderBufferSize;
    
    float mCurrentEV100 { 10.f };
    float mIblLuminanceMultiplier { 1000.f };

public:
    float mReflectionStepSize { 0.1f };
    int mReflectionMaxStepCount { 300 };
    float mReflectionThicknessThreshold { 1.2f };
    int mReflectionBinarySearchDepth { 10 };
    float mRoughnessFallOff { 20.f };
    float mReflectionDivideSize { 2.f };
};
