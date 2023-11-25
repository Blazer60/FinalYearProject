/**
 * @file RendererSettings.cpp
 * @author Ryan Purse
 * @date 25/11/2023
 */


#include "RendererSettings.h"

#include <imgui.h>
#include <GraphicsState.h>

void drawRendererSettings()
{
    ImGui::DragFloat("Step Size", &graphics::renderer->mReflectionStepSize, 0.001f);
    ImGui::DragInt("Step Count", &graphics::renderer->mReflectionMaxStepCount);
    ImGui::DragFloat("Thickness", &graphics::renderer->mReflectionThicknessThreshold, 0.01f);
    ImGui::DragInt("Binary Search Depth", &graphics::renderer->mReflectionBinarySearchDepth);
    ImGui::DragFloat("Depth Falloff", &graphics::renderer->mRoughnessFallOff);
    ImGui::DragFloat("Reflection Resolution Size", &graphics::renderer->mReflectionDivideSize);
}
