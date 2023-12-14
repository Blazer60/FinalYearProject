/**
 * @file GameInput.h
 * @author Ryan Purse
 * @date 14/12/2023
 */


#pragma once

#include "EventHandler.h"
#include "Pch.h"

extern class GameInput *gameInput;

/**
 * @author Ryan Purse
 * @date 14/12/2023
 */
class GameInput
    : public engine::event::EventHandler
{
public:
    void update() override;

    Value onMoveForward { {  { ImGuiKey_W, -1.f }, { ImGuiKey_S, 1.f } } };
    Value onMoveRight   { {  { ImGuiKey_A, -1.f }, { ImGuiKey_D, 1.f } } };
};
