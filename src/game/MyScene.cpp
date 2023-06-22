/**
 * @file MyScene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "MyScene.h"
#include "Renderer.h"
#include "Shader.h"
#include "Vertices.h"
#include "ModelLoader.h"

MyScene::MyScene()
    : mMainCamera(glm::vec3(0.f, 8.f, 30.f))
{
    const auto [mesh, simpleMaterials] = load::model<StandardVertex, StandardMaterial>(
        "../resources/models/pillars/Pillars.obj",
        std::make_shared<Shader>(
            "../resources/shaders/geometry/standard/Standard.vert",
            "../resources/shaders/geometry/standard/Standard.frag"));
    
    mMesh = mesh;
    mMaterials = simpleMaterials;
}

void MyScene::onFixedUpdate()
{
}

void MyScene::onUpdate()
{
    mMainCamera.update();
}

void MyScene::onRender()
{
    renderer::submit(CameraSettings(mMainCamera.getProjectionMatrix(), mMainCamera.getViewMatrix()));
    renderer::drawMesh(mMesh, mMaterials, glm::mat4(1.f));
}

void MyScene::onImguiUpdate()
{
    mMainCamera.imguiUpdate();
}

void MyScene::onImguiMenuUpdate()
{
}

MyScene::~MyScene()
{
}

