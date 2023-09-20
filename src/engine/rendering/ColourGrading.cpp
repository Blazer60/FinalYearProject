/**
 * @file ColourGrading.cpp
 * @author Ryan Purse
 * @date 14/07/2023
 */


#include "ColourGrading.h"
#include "imgui.h"
#include "GraphicsState.h"

void ColourGrading::onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput)
{
    glViewport(0, 0, imageOutput->getSize().x, imageOutput->getSize().y);
    
    mFramebuffer->bind();
    mFramebuffer->attach(imageOutput, 0);

    mShader.bind();
    mShader.set("u_texture", imageInput->getId(), 0);
    mShader.set("u_inv_gamma_correction", 1.f / 2.2f);
    mShader.set("u_exposure", covertEV100ToExposure());
    
    graphics::renderer->drawFullscreenTriangleNow();
    
    mFramebuffer->detach(0);
}

void ColourGrading::onDrawUi()
{
    if (ImGui::TreeNode("Colour Grading"))
    {
        ImGui::DragFloat("EV100", &mManualEV100, 0.01f);
        ImGui::TreePop();
    }
}

float ColourGrading::covertEV100ToExposure() const
{
    float maxLuminance = 1.2f * glm::pow(2.f, mManualEV100);
    return 1.f / maxLuminance;
}
