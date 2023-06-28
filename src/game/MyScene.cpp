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
#include "UiHelpers.h"
#include "imgui.h"

MyScene::MyScene()
    : mMainCamera(glm::vec3(0.f, 8.f, 30.f)), mDirectionalLight(glm::normalize(glm::vec3(-1.f, 1.f, -1.f)), glm::vec3(0.93f, 0.93f, 0.95f) , glm::ivec2(2048))
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
    renderer::submit(mDirectionalLight);
    renderer::drawMesh(mMesh, mMaterials, glm::mat4(1.f));
}

void MyScene::onImguiUpdate()
{
    mMainCamera.imguiUpdate();
    bool show = true;
    ui::showTextureBuffer("Light Shadow Map", *mDirectionalLight.shadowMap, &show, false);
    
    ImGui::DragFloat("Left", &mLeft);
    ImGui::DragFloat("Right", &mRight);
    ImGui::DragFloat("Top", &mTop);
    ImGui::DragFloat("Bottom", &mBottom);
    ImGui::DragFloat("Near", &mNear);
    ImGui::DragFloat("Far", &mFar);
    
    mDirectionalLight.projectionMat = glm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
}

void MyScene::onImguiMenuUpdate()
{
}

MyScene::~MyScene()
{
}

