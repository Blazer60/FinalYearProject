/**
 * @file Materials.h
 * @author Ryan Purse
 * @date 15/06/2023
 */


#pragma once

#include <utility>

#include "Pch.h"
#include "Shader.h"
#include "RendererHelpers.h"
#include "Texture.h"

struct MtlMaterialInformation
{
    unsigned int    nS      { 0 };
    glm::vec3       kA      { 0.f };
    glm::vec3       kD      { 0.f };
    glm::vec3       kS      { 0.f };
    unsigned int    d       { 1 };
    unsigned int    illum   { 2 };
    std::string     mapKd   { "" };
    std::string     mapKe   { "" };
    std::string     mapNs   { "" };
};

class Material;

typedef std::vector<std::shared_ptr<Material>> SharedMaterials;

class Material
{
public:
    explicit Material(renderer::DrawMode drawMode=renderer::Triangles)
        : mDrawMode(drawMode) {};
    
    [[nodiscard]] renderer::DrawMode drawMode() const { return mDrawMode; }
    [[nodiscard]] std::weak_ptr<Shader> shader() const { return mShader; }
    
    void attachShader(std::shared_ptr<Shader> shader);
    
    virtual void onDraw() {};
    virtual void onLoadMtlFile(const MtlMaterialInformation &materialInformation) {};
    
protected:
    std::shared_ptr<Shader> mShader = nullptr;
    renderer::DrawMode mDrawMode;
};

class SimpleMaterial
    : public Material
{
public:
    void onLoadMtlFile(const MtlMaterialInformation &materialInformation) override;
    void onDraw() override;
    
protected:
    glm::vec3 mColour { 1.f, 0.f, 1.f };
    std::shared_ptr<Texture> mTexture;
};
