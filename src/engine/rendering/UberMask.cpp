/**
 * @file UberMask.cpp
 * @author Ryan Purse
 * @date 14/03/2024
 */


#include "UberMask.h"

#include "Ui.h"

namespace engine
{
    void UberMask::onDrawUi()
    {
        ImGui::PushID("Masking Layer");
        if (ImGui::BeginTable("Mask Layer Table", 4))
        {
            ImGui::TableSetupColumn("Texture Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Close Button", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed, ui::buttonSize().x);

            ui::rowTextureSliderFloat("Mask Alpha", mMaskTexture, mAlphaThreshold);

            ImGui::EndTable();
        }
        ImGui::PopID();
    }
}
