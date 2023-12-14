/**
 * @file GameInput.cpp
 * @author Ryan Purse
 * @date 14/12/2023
 */


#include "GameInput.h"

GameInput *gameInput;

void GameInput::update()
{
    onMoveForward.doAction();
    onMoveRight.doAction();
    onJump.doAction();
}
