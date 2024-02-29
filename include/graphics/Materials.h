/**
 * @file Materials.h
 * @author Ryan Purse
 * @date 15/06/2023
 */


#pragma once

#include <utility>

#include "Pch.h"
#include "Shader.h"
#include "GraphicsDefinitions.h"
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
    std::string     mapBump { "" };
};

class Material;

typedef std::vector<std::shared_ptr<Material>> SharedMaterials;

class Material
{
public:
    virtual ~Material() = default;

    explicit Material(graphics::drawMode drawMode=graphics::drawMode::Triangles)
        : mDrawMode(drawMode) {};
    
    [[nodiscard]] graphics::drawMode drawMode() const { return mDrawMode; }
    [[nodiscard]] std::weak_ptr<Shader> shader() const { return mShader; }
    
    void attachShader(std::shared_ptr<Shader> shader);
    
    virtual void onDraw() {};
    virtual void onLoadMtlFile(const MtlMaterialInformation &materialInformation) {}
    
protected:
    std::shared_ptr<Shader> mShader = nullptr;
    graphics::drawMode mDrawMode;
};


/**
 * @brief For use with Standard.vert and Standard.frag shaders.
 */
class StandardMaterial
    : public Material
{
public:
    StandardMaterial();
    void onDraw() override;
    void onLoadMtlFile(const MtlMaterialInformation &materialInformation) override;
    
    void setDiffuseMap(std::shared_ptr<Texture> diffuseMap);
    void setSpecularMap(std::shared_ptr<Texture> specularMap);
    void setNormalMap(std::shared_ptr<Texture> normalMap);
    void setHeightMap(std::shared_ptr<Texture> heightMap);
    void setRoughnessMap(std::shared_ptr<Texture> roughnessMap);
    void setMetallicMap(std::shared_ptr<Texture> metallicMap);
    
    [[nodiscard]] uint32_t diffuseMapId() const { return mDiffuse->id(); }
    [[nodiscard]] uint32_t specularMapId() const { return mSpecular->id(); }
    [[nodiscard]] uint32_t normalMapId() const { return mNormal->id(); }
    [[nodiscard]] uint32_t heightMapId() const { return mHeight->id(); }
    [[nodiscard]] uint32_t roughnessMapId() const { return mRoughnessMap->id(); }
    [[nodiscard]] uint32_t metallicMapId() const { return mMetallicMap->id(); }


    float heightScale { 0.1f };
    int32_t maxHeightSamples { 64 };
    int32_t minHeightSamples { 8 };
    float roughness { 1.f };
    float metallic { 0 };
    glm::vec3 diffuseColour { 1.f };
    glm::vec3 specularColour { 0.f };
    glm::vec3 emissive { 0.f };
    
protected:
    std::shared_ptr<Texture> mDiffuse;
    std::shared_ptr<Texture> mSpecular;
    std::shared_ptr<Texture> mNormal;
    std::shared_ptr<Texture> mHeight;
    std::shared_ptr<Texture> mRoughnessMap;
    std::shared_ptr<Texture> mMetallicMap;
};
