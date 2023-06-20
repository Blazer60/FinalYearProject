/**
 * @file Renderer.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include "Shader.h"
#include "CameraMatrices.h"
#include "SubMesh.h"
#include "Materials.h"
#include "RendererHelpers.h"
#include <functional>

namespace renderer
{
    bool init();
    
    void submit(
        uint32_t vao, int32_t indicesCount,
        std::weak_ptr<Shader> shader, DrawMode renderMode,
        const DrawCallback &onDraw);
    
    void submit(
        const SubMesh &subMesh, std::weak_ptr<Shader> shader,
        DrawMode renderMode, const DrawCallback &onDraw);
    
    void submit(const CameraMatrices &cameraMatrices);
    
    template<typename TMaterial>
    void submit(const SubMesh &subMesh, TMaterial &material, std::weak_ptr<Shader> shader);
    
    void render();
    
    /**
     * @returns false if debug message was failed to be setup. This is most likely due to the openGl version being
     * less than 4.3.
     * @see https://docs.gl/gl4/glDebugMessageCallback
     */
    bool debugMessageCallback(GLDEBUGPROC callback);
    
    [[nodiscard]] std::string getVersion();
    
    template<typename TMaterial>
    void submit(const SubMesh &subMesh, TMaterial &material, std::weak_ptr<Shader> shader)
    {
        submit(subMesh.vao(), subMesh.indicesCount(), shader, material.drawMode(),
               [&](Shader & shader, const CameraMatrices & cameraMatrices) {
            material.OnDraw(shader, cameraMatrices);
        });
    }
}
