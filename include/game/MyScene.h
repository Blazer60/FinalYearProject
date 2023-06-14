/**
 * @file MyScene.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"

#include "Scene.h"
#include "Shader.h"

/**
 * @author Ryan Purse
 * @date 13/06/2023
 */
class MyScene
    : public engine::Scene
{
public:
    MyScene();
    ~MyScene() override;
    void onFixedUpdate() override;
    void onUpdate() override;
    void onRender() override;
    void onImguiUpdate() override;
    void onImguiMenuUpdate() override;
    
protected:
    uint32_t mVbo { 0 };
    uint32_t mEbo { 0 };
    int32_t mEboCount { 0 };
    uint32_t mVao { 0 };
    std::shared_ptr<Shader> mShader ;
};
