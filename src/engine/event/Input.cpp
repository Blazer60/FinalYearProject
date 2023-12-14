/**
 * @file Action.cpp
 * @author Ryan Purse
 * @date 23/08/2023
 */


#include "Input.h"

Button::Button(ImGuiKey key)
{
    mKeys.emplace_back(key);
}

void Button::doAction()
{
    for (const ImGuiKey &key : mKeys)
    {
        if (ImGui::IsKeyPressed(key, false))
        {
            broadcast();
            return;
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
