/**
 * @file MeshRenderer.h
 * @author Ryan Purse
 * @date 17/10/2023
 */


#pragma once

#include "Pch.h"
#include "Component.h"
#include "Mesh.h"
#include "Materials.h"
#include "AssimpLoader.h"
#include "MaterialSubComponent.h"
#include "EngineMemory.h"

namespace engine
{
/**
 * @author Ryan Purse
 * @date 17/10/2023
 */
    class MeshRenderer
        : public Component
    {
    public:
        MeshRenderer(std::vector<std::shared_ptr<SubMesh>> &&mesh, std::string path);
        void addMaterial(const std::shared_ptr<MaterialSubComponent> &material);
        
    protected:
        void onDrawUi() override;
        void onPreRender() override;
        void drawMeshOptions();
        
        // An ideal world with have all of these with an equal length. Some mesh renderers can have multiple meshes
        // but only a single material.
        std::vector<std::shared_ptr<SubMesh>>               mMeshes;
        std::vector<std::shared_ptr<MaterialSubComponent>>  mMaterials;
        
        bool mIsShowing { true };
        
        std::string mMeshPath;
    };
} // engine

namespace load
{
    /**
     * @brief Loads a model and puts it into a mesh renderer.
     * @tparam TVertex - The type of vertex used to construct the model.
     * @param path - the path to the model.
     * @returns A mesh renderer as a resource.
     */
    template<typename TVertex>
    Resource<engine::MeshRenderer> meshRenderer(const std::filesystem::path &path);
    
    template<typename TVertex>
    Resource<engine::MeshRenderer> meshRenderer(const std::filesystem::path &path)
    {
        return makeResource<engine::MeshRenderer>(load::model<TVertex>(path), path.string());
    }
    
}
