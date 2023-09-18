/**
 * @file Renderer.h
 * @author Ryan Purse
 * @date 14/07/2023
 */


#pragma once

#include "Pch.h"
#include "CameraSettings.h"
#include "Shader.h"
#include "GraphicsDefinitions.h"
#include "Mesh.h"
#include "Materials.h"
#include "Lighting.h"
#include "PostProcessLayer.h"
#include "Buffers.h"


/**
 * @author Ryan Purse
 * @date 14/07/2023
 */
class Renderer
{
public:
    Renderer();
    Renderer(const Renderer &r) = delete;
    Renderer(Renderer &&r) = delete;
    
    /**
     * @brief Draws an element to the geometry buffer.
     * @param vao - Vertex Array Object.
     * @param indicesCount - The number of indices that make up the geometry.
     * @param shader - The shader used to drawUi the element to the geometry buffer.
     * @param renderMode - What primitive to use when rendering.
     * @param matrix - The modelAndMaterial matrix for this object (used for shadow mapping).
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
     * @param matrix - The modelAndMaterial's matrix (used for shadow mapping).
     */
    void drawMesh(const SubMesh &subMesh, Material &material, const glm::mat4 &matrix);
    
    /**
     * @brief Draws a number of meshes to the geometry buffer with the same modelAndMaterial matrix.
     * @param mesh - N sub-meshes that you want to be drawn to the geometry buffer.
     * @param materials - N materials used to drawUi each mesh. If the material count is one, then only that material
     * will be used.
     * @param matrix - the modelAndMaterial's matrix (used for shadow mapping).
     */
    void drawMesh(const SharedMesh &mesh, const SharedMaterials &materials, const glm::mat4 &matrix);
    
    /**
     * @brief Submits a camera that will be used for the render pipeline.
     */
    void submit(const CameraSettings &cameraSettings);
    
    /**
     * @brief Submits a directional light that will be used to light the world and cast shadows.
     */
    void submit(const DirectionalLight &directionalLight);
    
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
    
    [[nodiscard]] static std::string getVersion();
    
    void generateSkybox(std::string_view path, const glm::ivec2 desiredSize);
    
    /**
     * @brief Draws a triangle to the screen so that fullscreen passes can be performs without
     * having to worry about mesh data. Make sure that a shader and fbo are already bound before
     * calling this method.
     */
    void drawFullscreenTriangleNow();
    
    [[nodiscard]] const TextureBufferObject &getPrimaryBuffer();
    [[nodiscard]] const TextureBufferObject &getDeferredLightingBuffer();
    [[nodiscard]] const TextureBufferObject &getAlbedoBuffer();
    [[nodiscard]] const TextureBufferObject &getNormalBuffer();
    [[nodiscard]] const TextureBufferObject &getPositionBuffer();
    [[nodiscard]] const TextureBufferObject &getEmissiveBuffer();
    [[nodiscard]] const TextureBufferObject &getLightBuffer();
    [[nodiscard]] const TextureBufferObject &getDepthBuffer();
    [[nodiscard]] const TextureBufferObject &getShadowBuffer();
    [[nodiscard]] const TextureBufferObject &getRoughnessBuffer();
    [[nodiscard]] const TextureBufferObject &getMetallicBuffer();
    
    [[nodiscard]] std::vector<DirectionalLight> &getDirectionalLights();
    
public:
    /**
     * @brief Upon initialisation, if false, then the renderer will not work properly and the application must be
     * shutdown immediately.
     */
    bool isOk { true };
    
    float shadowZMultiplier { 5.f };
    std::vector<float> shadowCascadeMultipliers { 0.04f, 0.16f, 0.36f, 0.64f };
    uint32_t shadowCascadeZones { 5 };
    glm::vec2 shadowBias { 0.001f, 0.f };
    
protected:
    void initFrameBuffers();
    void initTextureRenderBuffers();
    void detachTextureRenderBuffersFromFrameBuffers();
    std::unique_ptr<Cubemap> createCubemapFromHdrTexture(HdrTexture *hdrTexture, const glm::ivec2 &size);
    std::unique_ptr<Cubemap> generateIrradianceMap(Cubemap *cubemap, const glm::ivec2 &size);
    std::unique_ptr<Cubemap> generatePreFilterMap(Cubemap *cubemap, const glm::ivec2 &size);
    std::unique_ptr<TextureBufferObject> generateBrdfLut(const glm::ivec2 &size);
    void shadowMapping(const CameraSettings &cameraSettings, const std::vector<float> &cascadeDepths);
    
protected:
    std::vector<graphics::RenderQueueObject>        mRenderQueue;
    std::vector<CameraSettings>                     mCameraQueue;
    std::vector<DirectionalLight>                   mDirectionalLightQueue;
    
    std::unique_ptr<HdrTexture>  mHdrImage;
    std::unique_ptr<Cubemap>     mHdrSkybox;
    std::unique_ptr<Cubemap>     mIrradianceMap;
    std::unique_ptr<Cubemap>     mPreFilterMap;
    
    std::unique_ptr<FramebufferObject> mDeferredLightFramebuffer;
    std::unique_ptr<FramebufferObject> mGeometryFramebuffer;
    std::unique_ptr<FramebufferObject> mLightFramebuffer;
    std::unique_ptr<FramebufferObject> mShadowFramebuffer;
    
    std::unique_ptr<Shader> mDeferredLightShader;
    std::unique_ptr<Shader> mDirectionalLightShader;
    std::unique_ptr<Shader> mIblShader;
    std::unique_ptr<Shader> mShadowShader;
    std::unique_ptr<Shader> mHdrToCubemapShader;
    std::unique_ptr<Shader> mCubemapToIrradianceShader;
    std::unique_ptr<Shader> mPreFilterShader;
    std::unique_ptr<Shader> mIntegrateBrdfShader;
    
    SubMesh mFullscreenTriangle;
    
    std::unique_ptr<TextureBufferObject> mAlbedoTextureBuffer;
    std::unique_ptr<TextureBufferObject> mDepthTextureBuffer;
    std::unique_ptr<TextureBufferObject> mLightTextureBuffer;
    std::unique_ptr<TextureBufferObject> mEmissiveTextureBuffer;
    std::unique_ptr<TextureBufferObject> mNormalTextureBuffer;
    std::unique_ptr<TextureBufferObject> mRoughnessTextureBuffer;
    std::unique_ptr<TextureBufferObject> mMetallicTextureBuffer;
    std::unique_ptr<TextureBufferObject> mDeferredLightingTextureBuffer;
    std::unique_ptr<TextureBufferObject> mPositionTextureBuffer;
    std::unique_ptr<TextureBufferObject> mShadowTextureBuffer;
    std::unique_ptr<TextureBufferObject> mBrdfLutTextureBuffer;
    std::unique_ptr<TextureBufferObject> mPrimaryImageBuffer;
    std::unique_ptr<TextureBufferObject> mAuxiliaryImageBuffer;
    
    glm::ivec2 mCurrentRenderBufferSize;
};
