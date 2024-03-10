/**
 * @file ColourGrading.cpp
 * @author Ryan Purse
 * @date 14/07/2023
 */


#include "ColourGrading.h"
#include "imgui.h"
#include "GraphicsState.h"
#include "ProfileTimer.h"
#include "GraphicsFunctions.h"

void ColourGrading::onDraw(TextureBufferObject *imageInput, TextureBufferObject *imageOutput, graphics::Context *context)
{
    PROFILE_FUNC();
    graphics::pushDebugGroup("Colour Grading Pass");
    
    glViewport(0, 0, imageOutput->getSize().x, imageOutput->getSize().y);
    
    mFramebuffer->bind();
    mFramebuffer->attach(imageOutput, 0);

    mShader.bind();
    mShader.set("u_texture", imageInput->getId(), 0);

    graphics::renderer->drawFullscreenTriangleNow();
    
    mFramebuffer->detach(0);
    
    graphics::popDebugGroup();
}

void ColourGrading::onDrawUi()
{
    if (ImGui::TreeNode("Colour Grading"))
    {
        ImGui::TreePop();
    }
}
