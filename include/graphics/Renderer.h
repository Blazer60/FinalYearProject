/**
 * @file Renderer.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include "CameraSettings.h"
#include "Mesh.h"
#include "Materials.h"
#include "RendererHelpers.h"
#include "TextureBufferObject.h"
#include "Lighting.h"
#include <functional>

namespace renderer
{
    bool init();
    
    void drawMesh(
        uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader, DrawMode renderMode, const glm::mat4 &matrix,
        const DrawCallback &onDraw);
    
    void drawMesh(const SubMesh &subMesh, Material &material, const glm::mat4 &matrix);
    void drawMesh(const SharedMesh &mesh, const SharedMaterials &materials, const glm::mat4 &matrix);
    
    void submit(const CameraSettings &cameraMatrices);
    void submit(const DirectionalLight &directionalLight);
    
    void render();
    
    [[nodiscard]] const TextureBufferObject &getOutputBuffer();
    [[nodiscard]] const TextureBufferObject &getAlbedoBuffer();
    [[nodiscard]] const TextureBufferObject &getNormalBuffer();
    [[nodiscard]] const TextureBufferObject &getPositionBuffer();
    [[nodiscard]] const TextureBufferObject &getEmissiveBuffer();
    [[nodiscard]] const TextureBufferObject &getDiffuseBuffer();
    [[nodiscard]] const TextureBufferObject &getSpecularBuffer();
    
    
    /**
     * @returns false if debug message was failed to be setup. This is most likely due to the openGl version being
     * less than 4.3.
     * @see https://docs.gl/gl4/glDebugMessageCallback
     */
    bool debugMessageCallback(GLDEBUGPROC callback);
    
    [[nodiscard]] std::string getVersion();
    
// Private:
    void initFrameBuffers();
    void initTextureRenderBuffers();
    void detachTextureRenderBuffersFromFrameBuffers();
}
