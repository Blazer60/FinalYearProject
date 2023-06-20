/**
 * @file Renderer.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include "CameraSettings.h"
#include "SubMesh.h"
#include "Materials.h"
#include "RendererHelpers.h"
#include <functional>

namespace renderer
{
    bool init();
    
    void submit(
        uint32_t vao, int32_t indicesCount, std::weak_ptr<Shader> shader, DrawMode renderMode, const glm::mat4 &matrix,
        const DrawCallback &onDraw);
    
    void submit(const CameraSettings &cameraMatrices);
    
    template<typename TMaterial>
    void submit(const SubMesh &subMesh, TMaterial &material, const glm::mat4 &matrix, std::weak_ptr<Shader> shader);
    
    void render();
    
    /**
     * @returns false if debug message was failed to be setup. This is most likely due to the openGl version being
     * less than 4.3.
     * @see https://docs.gl/gl4/glDebugMessageCallback
     */
    bool debugMessageCallback(GLDEBUGPROC callback);
    
    [[nodiscard]] std::string getVersion();
    
    template<typename TMaterial>
    void submit(const SubMesh &subMesh, TMaterial &material, const glm::mat4 &matrix, std::weak_ptr<Shader> shader)
    {
        submit(
            subMesh.vao(), subMesh.indicesCount(), shader, material.drawMode(), matrix,
            [&](Shader &shader) {
                material.OnDraw(shader);
            }
        );
    }
}
