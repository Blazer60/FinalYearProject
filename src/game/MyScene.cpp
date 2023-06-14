/**
 * @file MyScene.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "MyScene.h"
#include "Renderer.h"
#include "Shader.h"

MyScene::MyScene()
    : mMainCamera(glm::vec3(0.f, 0.f, 2.f))
{
    std::vector<float> triangleData {
         0.f,   0.5f, 0.f,
        -0.5f, -0.5f, 0.f,
         0.5f, -0.5f, 0.f,
    };
    std::vector<uint32_t> triangleIndices { 0, 1, 2 };
    
    glCreateBuffers(1, &mVbo);
    glCreateBuffers(1, &mEbo);
    glNamedBufferData(mVbo, static_cast<int64_t>(triangleData.size() * sizeof(float)), static_cast<const void *>(&triangleData[0]), GL_STATIC_DRAW);
    glNamedBufferData(mEbo, static_cast<int64_t>(triangleIndices.size() * sizeof(uint32_t)), static_cast<const void *>(&triangleIndices[0]), GL_STATIC_DRAW);
    mEboCount = static_cast<int32_t>(triangleIndices.size());
    
    glCreateVertexArrays(1, &mVao);
    
    const unsigned int bindingIndex = 0;
    const unsigned int offSet = 0;
    const unsigned int stride = sizeof(float) * 3;  // Because each vertex is 3 floats.
    
    glEnableVertexArrayAttrib(mVao, 0);
    glVertexArrayAttribFormat(mVao, 0, 3, GL_FLOAT, GL_FALSE, offSet);
    
    glVertexArrayVertexBuffer(mVao, bindingIndex, mVbo, offSet, stride);
    glVertexArrayElementBuffer(mVao, mEbo);
    
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
    renderer::submit(CameraMatrices(mMainCamera.getProjectionMatrix(), mMainCamera.getViewMatrix()));
    renderer::submit(mVao, mEboCount, mShader, renderer::Triangles, [](Shader &shader, const CameraMatrices &cameraMatrices){
        shader.set("u_colour", glm::vec3(0.f, 1.f, 0.f));
        shader.set("u_mvp_matrix", cameraMatrices.getVpMatrix());
    });
}

void MyScene::onImguiUpdate()
{
}

void MyScene::onImguiMenuUpdate()
{
}

MyScene::~MyScene()
{
    glDeleteBuffers(1, &mVbo);
    glDeleteBuffers(1, &mEbo);
    
    glDeleteVertexArrays(1, &mVao);
}

