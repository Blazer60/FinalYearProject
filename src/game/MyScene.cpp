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
    : mMainCamera(glm::vec3(0.f, 0.f, 2.f))
{
    const auto [mesh, simpleMaterials] = load::model<PositionVertex, SimpleMaterial>(
        "../resources/models/utahTeapot/UtahTeapot.obj",
        std::make_shared<Shader>(
            "../resources/shaders/Triangle.vert",
            "../resources/shaders/Triangle.frag"));
    
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
    renderer::submit(mMesh, mMaterials, glm::mat4(1.f));
}

void MyScene::onImguiUpdate()
{
}

void MyScene::onImguiMenuUpdate()
{
}

MyScene::~MyScene()
{
}

