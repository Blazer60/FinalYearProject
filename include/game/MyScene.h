/**
 * @file MyScene.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Audio.h"
#include "AudioSource.h"
#include "FileLoader.h"
#include "Pch.h"

#include "Scene.h"

/**
 * @author Ryan Purse
 * @date 13/06/2023
 */
class MyScene
    : public engine::Scene
{
    SERIALIZABLE_SCENE(MyScene);
public:
    MyScene();
    ~MyScene() override;
    void onFixedUpdate() override;
    void onUpdate() override;
    void onRender() override;
    void onImguiUpdate() override;
    void onImguiMenuUpdate() override;

    void setLuminanceMultiplier(float multiplier);
    
protected:
    float mLuminanceMultiplier { 1000.f };
};
