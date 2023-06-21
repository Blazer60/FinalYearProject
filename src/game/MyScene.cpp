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
    mMesh = load::model<PositionVertex>("../resources/models/utahTeapot/UtahTeapot.obj").mesh;
    mShader = std::make_shared<Shader>("../resources/shaders/Triangle.vert", "../resources/shaders/Triangle.frag");
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
    renderer::submit(mMesh, mSimpleMaterial, glm::mat4(1.f), mShader);
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

