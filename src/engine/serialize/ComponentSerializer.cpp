/**
 * @file ComponentSerializer.cpp
 * @author Ryan Purse
 * @date 15/10/2023
 */


#include "ComponentSerializer.h"
#include <Statistics.h>
#include "yaml-cpp/emitter.h"
#include "EngineState.h"
#include "Component.h"
#include "MeshRenderer.h"
#include "FileLoader.h"
#include "Lighting.h"
#include "Actor.h"
#include "Core.h"
#include "ShaderLoader.h"

namespace engine
{
    void attachComponentSerialization()
    {
        serializer->pushSaveComponent<MeshRenderer>();
        serializer->pushSaveComponent<DirectionalLight>();
        serializer->pushSaveComponent<PointLight>();
        
        serializer->pushLoadComponent("MeshRenderer", [](const YAML::Node &node, Ref<Actor> actor) {
            const std::filesystem::path relativePath = node["MeshPath"].as<std::string>();
            Ref<engine::MeshRenderer> meshRenderer = actor->addComponent(
                load::meshRenderer<StandardVertex>(file::resourcePath() / relativePath));
            
            for (const auto &materialNode : node["Materials"])
            {
                const auto materialType = materialNode["MaterialType"].as<std::string>();
                
                if (materialType == "StandardMaterial")
                {
                    auto standardMaterial = std::make_shared<engine::StandardMaterialSubComponent>();
                    standardMaterial->attachShader(load::shader(
                        file::shaderPath() / "geometry/standard/Standard.vert",
                        file::shaderPath() / "geometry/standard/Standard.frag"));
                    meshRenderer->addMaterial(standardMaterial);
                    
                    standardMaterial->setAmbientColour(materialNode["Albedo"].as<glm::vec3>());
                    standardMaterial->setRoughness(materialNode["Roughness"].as<float>());
                    standardMaterial->setMetallic(materialNode["Metallic"].as<float>());
                    standardMaterial->setEmissive(materialNode["Emissive"].as<glm::vec3>());
                    standardMaterial->setHeightScale(materialNode["HeightScale"].as<float>());
                    
                    const auto diffuseRelativePath = materialNode["DiffuseMap"].as<std::string>();
                    if (!diffuseRelativePath.empty())
                        standardMaterial->setDiffuseMap(file::resourcePath() / diffuseRelativePath);
                    
                    const auto normalRelativePath = materialNode["NormalMap"].as<std::string>();
                    if (!normalRelativePath.empty())
                        standardMaterial->setNormalMap(file::resourcePath() / normalRelativePath);
                    
                    const auto heightRelativePath = materialNode["HeightMap"].as<std::string>();
                    if (!heightRelativePath.empty())
                        standardMaterial->setHeightMap(file::resourcePath() / heightRelativePath);
                    
                    const auto roughnessRelativePath = materialNode["RoughnessMap"].as<std::string>();
                    if (!roughnessRelativePath.empty())
                        standardMaterial->setRoughnessMap(file::resourcePath() / roughnessRelativePath);
                    
                    const auto metallicRelativePath = materialNode["MetallicMap"].as<std::string>();
                    if (!metallicRelativePath.empty())
                        standardMaterial->setMetallicMap(file::resourcePath() / metallicRelativePath);
                }
            }
        });
        
        serializer->pushLoadComponent("DirectionalLight", [](const YAML::Node &node, Ref<Actor> actor) {
            const auto colour = node["Colour"].as<glm::vec3>();
            const auto intensity = node["Intensity"].as<float>();
            const auto yaw = node["Yaw"].as<float>();
            const auto pitch = node["Pitch"].as<float>();
            
            const auto depthCount = node["CascadeDepths"].size();
            std::vector<float> depths;
            depths.reserve(depthCount);
            for (int i = 0; i < depthCount; ++i)
                depths.emplace_back(node["CascadeDepths"][i].as<float>());
            
            const auto zMultiplier = node["ZMultiplier"].as<float>();
            const auto bias = node["Bias"].as<glm::vec2>();
            
            actor->addComponent(makeResource<engine::DirectionalLight>(yaw, pitch, colour, intensity, depths, zMultiplier, bias));
        });
        
        serializer->pushLoadComponent("PointLight", [](const YAML::Node &node, Ref<Actor> actor) {
            const auto colour = node["Colour"].as<glm::vec3>();
            const auto intensity = node["Intensity"].as<float>();
            const auto radius = node["Radius"].as<float>();
            const auto bias = node["Bias"].as<glm::vec2>();
            const auto softness = node["Softness"].as<float>();
            const auto resolution = node["Resolution"].as<int>();
            
            actor->addComponent(makeResource<engine::PointLight>(colour, intensity, radius, bias, softness, resolution));
        });
    }
}

void serializeComponent(YAML::Emitter &out, engine::MeshRenderer *meshRenderer)
{
    out << YAML::Key << "Component" << YAML::Value << "MeshRenderer";
    out << YAML::Key << "MeshPath" << YAML::Value << file::makeRelativeToResourcePath(meshRenderer->mMeshPath).string();
    
    out << YAML::Key << "Materials" << YAML::Value << YAML::BeginSeq;
    for (const auto &iMaterial : meshRenderer->mMaterials)
    {
        out << YAML::BeginMap;

        if (auto standardMaterial = dynamic_cast<engine::StandardMaterialSubComponent*>(iMaterial.get());
            standardMaterial != nullptr)
        {
            out << YAML::Key << "MaterialType" << YAML::Value << "StandardMaterial";
            out << YAML::Key << "Albedo"       << YAML::Value << standardMaterial->mMaterial.ambientColour;
            out << YAML::Key << "Roughness"    << YAML::Value << standardMaterial->mMaterial.roughness;
            out << YAML::Key << "Metallic"     << YAML::Value << standardMaterial->mMaterial.metallic;
            out << YAML::Key << "Emissive"     << YAML::Value << standardMaterial->mMaterial.emissive;
            out << YAML::Key << "HeightScale" << YAML::Value << standardMaterial->mMaterial.heightScale;
            out << YAML::Key << "DiffuseMap"   << YAML::Value << file::makeRelativeToResourcePath(standardMaterial->mDiffuseMapPath).string();
            out << YAML::Key << "NormalMap"    << YAML::Value << file::makeRelativeToResourcePath(standardMaterial->mNormalMapPath).string();
            out << YAML::Key << "HeightMap"    << YAML::Value << file::makeRelativeToResourcePath(standardMaterial->mHeightMapPath).string();
            out << YAML::Key << "RoughnessMap" << YAML::Value << file::makeRelativeToResourcePath(standardMaterial->mRoughnessMapPath).string();
            out << YAML::Key << "MetallicMap"  << YAML::Value << file::makeRelativeToResourcePath(standardMaterial->mMetallicMapPath).string();
        }

        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
}


void serializeComponent(YAML::Emitter &out, engine::DirectionalLight *directionalLight)
{
    out << YAML::Key << "Component"         << YAML::Value << "DirectionalLight";
    out << YAML::Key << "Colour"            << YAML::Value << directionalLight->colour;
    out << YAML::Key << "Intensity"         << YAML::Value << directionalLight->intensity;
    out << YAML::Key << "Yaw"               << YAML::Value << directionalLight->yaw;
    out << YAML::Key << "Pitch"             << YAML::Value << directionalLight->pitch;
    
    out << YAML::Key << "CascadeDepths"     << YAML::Value << YAML::Flow << YAML::BeginSeq;
    for (const auto &multiplier : directionalLight->mDirectionalLight.shadowCascadeMultipliers)
        out << multiplier;
    out << YAML::EndSeq;
    
    out << YAML::Key << "ZMultiplier"       << YAML::Value << directionalLight->mDirectionalLight.shadowZMultiplier;
    out << YAML::Key << "Bias"              << YAML::Value << directionalLight->mDirectionalLight.shadowBias;
}

void serializeComponent(YAML::Emitter &out, engine::PointLight *pointLight)
{
    out << YAML::Key << "Component"     << YAML::Value << "PointLight";
    out << YAML::Key << "Colour"        << YAML::Value << pointLight->mColour;
    out << YAML::Key << "Intensity"     << YAML::Value << pointLight->mIntensity;
    out << YAML::Key << "Radius"        << YAML::Value << pointLight->mRadius;
    out << YAML::Key << "Bias"          << YAML::Value << pointLight->mBias;
    out << YAML::Key << "Softness"      << YAML::Value << pointLight->mSoftnessRadius;
    out << YAML::Key << "Resolution"    << YAML::Value << pointLight->mResolution;
}
