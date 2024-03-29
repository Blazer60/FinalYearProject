/**
 * @file Action.cpp
 * @author Ryan Purse
 * @date 23/08/2023
 */


#include "Input.h"

Button::Button(const ImGuiKey key, const ImGuiKey modifier)
    : mKey(key), mModifier(modifier)
{
}

void Button::doAction()
{
    if (mModifier != ImGuiMod_None)
    {
        if (ImGui::IsKeyDown(mModifier) && ImGui::IsKeyPressed(mKey, false))
            broadcast();
    }
    else
    {
        if (!( ImGui::IsKeyDown(ImGuiMod_Alt)
            || ImGui::IsKeyDown(ImGuiMod_Ctrl)
            || ImGui::IsKeyDown(ImGuiMod_Shift)
            || ImGui::IsKeyDown(ImGuiMod_Shortcut)
            || ImGui::IsKeyDown(ImGuiMod_Super)
            )
            && ImGui::IsKeyPressed(mKey, false))
        {
            broadcast();
        }
    }
}

void Value::doAction()
{
    bool anyKeyDownFlag = false;
    float value = 0.f;
    for (const EventKeyValue &keyValue : mKeyValues)
    {
        if (ImGui::IsKeyDown(keyValue.key))
        {
            value += keyValue.value;
            anyKeyDownFlag = true;
        }
    }
    
    if (anyKeyDownFlag)
        broadcast(value);
}

Value::Value(EventKeyValue keyValue)
{
    mKeyValues.push_back(keyValue);
}

Value::Value(const std::vector<EventKeyValue> &keyValues)
    : mKeyValues(keyValues)
{

}

void ScrollWheel::doAction()
{
    if (ImGui::IsKeyDown(ImGuiKey_MouseWheelY))
    {
        ImGuiIO &io = ImGui::GetIO();
        broadcast(io.MouseWheel);
    }
}
