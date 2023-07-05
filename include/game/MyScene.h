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
    std::shared_ptr<Shader> mStandardShader;
    DirectionalLight mDirectionalLight;
    
    SharedMesh mStoneFloorMesh;
    SharedMaterials mStoneFloorMaterial;
    
    std::shared_ptr<SubMesh> mBall;
    std::vector<std::shared_ptr<StandardMaterial>> mMaterials;
};
