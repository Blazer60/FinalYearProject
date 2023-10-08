/**
 * @file RendererImGui.cpp
 * @author Ryan Purse
 * @date 03/07/2023
 */


#include "RendererImGui.h"
#include "imgui.h"
#include "Ui.h"
#include "gtc/type_ptr.hpp"
#include "PostProcessLayer.h"
#include "GraphicsState.h"
#include "GraphicsFunctions.h"
#include "Buffers.h"
#include "ProfileTimer.h"

namespace graphics
{
    std::unique_ptr<TextureBufferObject> shadowMapViewer;
    int layer { 0 };
    
    void displayShadowMaps(TextureArrayObject *textureArrayObject)
    {
        PROFILE_FUNC();
        ImGui::SliderInt("Shadow map layer", &layer, 0, textureArrayObject->getLayerCount() - 1);
        shadowMapViewer = graphics::cloneTextureLayer(*textureArrayObject, layer);
        bool yes = true;
        ui::showTextureBuffer("Shadow Map", *shadowMapViewer, &yes, shadowMapViewer->getSize());
    }
}
