/**
 * @file SkyboxPass.h
 * @author Ryan Purse
 * @date 10/03/2024
 */


#pragma once

#include "Context.h"
#include "FileLoader.h"
#include "Pch.h"

namespace graphics
{
    class Skybox;

    /**
     * @author Ryan Purse
     * @date 10/03/2024
     */
    class SkyboxPass
    {
    public:
        void execute(const glm::ivec2& size, Context& context, const Skybox &skybox);

    protected:
        Shader mCombineLightingShader {
            { file::shaderPath() / "lighting/CombineOutput.comp" }
        };
    };

} // graphics
