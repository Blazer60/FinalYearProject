/**
 * @file Skybox.h
 * @author Ryan Purse
 * @date 10/03/2024
 */


#pragma once

#include "Cubemap.h"
#include "FileLoader.h"
#include "HdrTexture.h"
#include "Pch.h"

namespace graphics
{
    /**
     * @author Ryan Purse
     * @date 10/03/2024
     */
    class Skybox 
    {
    public:
        void generate(std::string_view path, glm::ivec2 size);

        Cubemap hdrSkybox = Cubemap(textureFormat::Rgba16f);
        Cubemap irradianceMap = Cubemap(textureFormat::Rgba16f);
        Cubemap prefilterMap = Cubemap(textureFormat::Rgba16f, 5);

        bool isValid = false;
        float luminanceMultiplier = 1000.f;

    protected:
        void createCubemapFromHdrTexture(const HdrTexture& hdrImage, glm::ivec2 size);
        void generateIrradianceMap(glm::ivec2 size);
        void generatePrefilterMap(glm::ivec2 size);

        Shader mHdrToCubemapShader {
            { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "cubemap/ToCubemap.frag" }
        };

        Shader mCubemapToIrradianceShader {
            { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "cubemap/IrradianceMap.frag" }
        };

        Shader mPreFilterShader {
            { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() /  "cubemap/PreFilter.frag" }
        };
    };
} // graphics
