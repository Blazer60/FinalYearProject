/**
 * @file MaterialSubComponent.h
 * @author Ryan Purse
 * @date 17/10/2023
 */


#pragma once

#include "Pch.h"
#include "Materials.h"
#include "Drawable.h"
#include "Serializer.h"

namespace engine
{
    class MeshRenderer;

/**
 * @author Ryan Purse
 * @date 17/10/2023
 */
    class MaterialSubComponent
        : public ui::Drawable
    {
    public:
        void attachShader(const std::shared_ptr<Shader> &shader);
        virtual Material &getMaterial() = 0;
    };
    
    class StandardMaterialSubComponent
        : public MaterialSubComponent
    {
        ENGINE_SERIALIZABLE_COMPONENT(MeshRenderer);
    public:
        StandardMaterialSubComponent() = default;
        
        void setDiffuseMap(const std::filesystem::path &diffuseMapPath);
        void setSpecularMap(const std::filesystem::path &specularMapPath);
        void setNormalMap(const std::filesystem::path &normalMapPath);
        void setHeightMap(const std::filesystem::path &heightMapPath);
        void setRoughnessMap(const std::filesystem::path &roughnessMapPath);
        void setMetallicMap(const std::filesystem::path &metallicMapPath);
        
        void setMetallic(float value);
        void setRoughness(float value);
        void setAmbientColour(const glm::vec3 &colour);
        void setSpecularColour(const glm::vec3 &colour);
        void setEmissive(const glm::vec3 &colour);
        void setHeightScale(float scale);
        
    protected:
        Material &getMaterial() override { return mMaterial; };
        void onDrawUi() override;
        
        StandardMaterial mMaterial;
        
        // Records for serialization.
        std::string mDiffuseMapPath;
        std::string mSpecularMapPath;
        std::string mNormalMapPath;
        std::string mHeightMapPath;
        std::string mRoughnessMapPath;
        std::string mMetallicMapPath;
    };
} // engine
