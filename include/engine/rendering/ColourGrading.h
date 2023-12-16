/**
 * @file ColourGrading.h
 * @author Ryan Purse
 * @date 14/07/2023
 */


#pragma once

#include "Pch.h"
#include "PostProcessLayer.h"
#include "Shader.h"
#include "Drawable.h"
#include <FileLoader.h>


/**
 * @author Ryan Purse
 * @date 14/07/2023
 */
class ColourGrading
    : public PostProcessLayer, public ui::Drawable
{
protected:
    void onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput) override;
    void onDrawUi() override;

    Shader mShader { file::shaderPath() / "FullscreenTriangle.vert", file::shaderPath() / "postProcessing/colourGrading/ColourGrading.frag" };
};
