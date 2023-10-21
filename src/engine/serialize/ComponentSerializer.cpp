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

namespace engine
{
    void attachComponentSerialization()
    {
        serializer->pushComponentDelegate([](YAML::Emitter &out, Component *component) -> bool {
            if (auto meshComponent = dynamic_cast<MeshRenderer*>(component); meshComponent != nullptr)
            {
                serializeComponent(out, meshComponent);
                return true;
            }
            return false;
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
