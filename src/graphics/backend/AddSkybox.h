/**
 * @file AddSkybox.h
 * @author Ryan Purse
 * @date 10/03/2024
 */


#pragma once

#include "Context.h"
#include "FileLoader.h"
#include "Pch.h"

namespace graphics {

    /**
     * @author Ryan Purse
     * @date 10/03/2024
     */
    class AddSkybox 
    {
    public:
        void execute(const glm::ivec2 &size, Context &context);

    protected:
        Shader mCombineLightingShader {
            { file::shaderPath() / "lighting/CombineOutput.comp" }
        };
    };

} // graphics
