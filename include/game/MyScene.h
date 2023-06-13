/**
 * @file MyScene.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"

#include "Scene.h"

/**
 * @author Ryan Purse
 * @date 13/06/2023
 */
class MyScene : public engine::Scene
{
public:
    void onFixedUpdate() override;
    
    void onUpdate() override;
    
    void onRender() override;
    
    void onImguiUpdate() override;
    
    void onImguiMenuUpdate() override;
};
