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
    out << YAML::Key << "MeshPath" << YAML::Value << meshRenderer->mMeshPath;
    
    out << YAML::Key << "Materials" << YAML::Value << YAML::BeginSeq;
    for (const auto &iMaterial : meshRenderer->mMaterials)
    {
        out << YAML::BeginMap;
        
        if (auto standardMaterial = dynamic_cast<engine::StandardMaterialSubComponent*>(iMaterial.get());
            standardMaterial != nullptr)
        {
            out << YAML::Key << "MaterialType" << YAML::Value << "StandardMaterial";
            out << YAML::Key << "Diffuse" << YAML::Value << standardMaterial->mDiffuseMapPath;
            out << YAML::Key << "Normal" << YAML::Value << standardMaterial->mNormalMapPath;
            out << YAML::Key << "Height" << YAML::Value << standardMaterial->mHeightMapPath;
            out << YAML::Key << "Roughness" << YAML::Value << standardMaterial->mRoughnessMapPath;
            out << YAML::Key << "Metallic" << YAML::Value << standardMaterial->mMetallicMapPath;
        }
        
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
}
