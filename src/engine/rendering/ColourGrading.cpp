/**
 * @file ColourGrading.cpp
 * @author Ryan Purse
 * @date 14/07/2023
 */


#include "ColourGrading.h"
#include "imgui.h"
#include "GraphicsState.h"
#include "FramebufferObject.h"

void ColourGrading::onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput)
{
    glViewport(0, 0, imageOutput->getSize().x, imageOutput->getSize().y);
    
    mFramebuffer->bind();
    mFramebuffer->attach(imageOutput, 0);

    mShader.bind();
    mShader.set("u_texture", imageInput->getId(), 0);
    mShader.set("u_inv_gamma_correction", 1.f / 2.2f);
    mShader.set("u_exposure", mExposure);
    mShader.set("u_slope", mSlope);
    
    graphics::renderer->drawFullscreenTriangleNow();
    
    mFramebuffer->detach(0);
}

void ColourGrading::onDrawUi()
{
    if (ImGui::TreeNode("Colour Grading"))
    {
        ImGui::DragFloat("Exposure", &mExposure, 0.001f);
        ImGui::SliderFloat("Slope", &mSlope, 0.f, 1.f);
        ImGui::TreePop();
    }
}
