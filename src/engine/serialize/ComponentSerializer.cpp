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

namespace engine
{
    // todo: Can this be inlined into the serializer?
    template<typename T>
    bool autoCast(YAML::Emitter &out, Component *component)
    {
        if (auto x = dynamic_cast<T*>(component); x != nullptr)
        {
            serializeComponent(out, x);
            return true;
        }
        return false;
    }
    
    void attachComponentSerialization()
    {
        serializer->pushComponentDelegate(autoCast<MeshRenderer>);
        serializer->pushComponentDelegate(autoCast<DirectionalLight>);
        serializer->pushComponentDelegate(autoCast<PointLight>);
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
