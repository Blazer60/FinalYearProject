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
#include "Drawable.h"

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
    : public ui::Drawable
{
public:
    explicit Material(graphics::drawMode drawMode=graphics::drawMode::Triangles)
        : mDrawMode(drawMode) {};
    
    [[nodiscard]] graphics::drawMode drawMode() const { return mDrawMode; }
    [[nodiscard]] std::weak_ptr<Shader> shader() const { return mShader; }
    
    void attachShader(std::shared_ptr<Shader> shader);
    
    virtual void onDraw() {};
    virtual void onLoadMtlFile(const MtlMaterialInformation &materialInformation) {}

protected:
    void onDrawUi() override;;
    
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
    void setNormalMap(std::shared_ptr<Texture> normalMap);
    void setHeightMap(std::shared_ptr<Texture> heightMap);
    void setRoughnessMap(std::shared_ptr<Texture> roughnessMap);

    float heightScale { 0.1f };
    int32_t maxHeightSamples { 64 };
    int32_t minHeightSamples { 8 };
    float roughness { 1.f };
    float metallic { 0 };
    glm::vec3 ambientColour { 1.f };
    
protected:
    void onDrawUi() override;

protected:
    std::shared_ptr<Texture> mDiffuse;
    std::shared_ptr<Texture> mNormal;
    std::shared_ptr<Texture> mHeight;
    std::shared_ptr<Texture> mRoughnessMap;
};

/**
 * @brief Performs a downcast to the base class material for an entire list.
 * @tparam TMaterial - The child class material that the list is made of.
 * @param materials - The vector containing all of the materials.
 * @returns A list shared materials that can be used by the renderer.
 */
template<typename TMaterial>
SharedMaterials toSharedMaterials(std::vector<std::shared_ptr<TMaterial>> materials)
{
    SharedMaterials sharedMaterials;
    for (const auto &material : materials)
        sharedMaterials.emplace_back(material);
    
    return sharedMaterials;
}
