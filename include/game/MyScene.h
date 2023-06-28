/**
 * @file MyScene.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"

#include "Scene.h"
#include "Shader.h"
#include "MainCamera.h"
#include "Mesh.h"
#include "Materials.h"
#include "Lighting.h"
#include "Cubemap.h"

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
    MainCamera mMainCamera;
    SharedMesh mMesh;
    SharedMaterials mMaterials;
    DirectionalLight mDirectionalLight;
    
    bool mShowOutput { true };
    
    float mLeft     { -80.0f };
    float mRight    {  80.0f };
    float mTop      {  80.0f };
    float mBottom   { -80.0f };
    float mNear     {   0.1f };
    float mFar      { 200.0f };
};
