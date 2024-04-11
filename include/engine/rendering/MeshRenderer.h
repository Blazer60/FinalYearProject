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
#include "MaterialSubComponent.h"
#include "EngineMemory.h"
#include "ComponentSerializer.h"
#include "Loader.h"

namespace engine
{
/**
 * @author Ryan Purse
 * @date 17/10/2023
 */
    class MeshRenderer
        : public Component
    {
        ENGINE_SERIALIZABLE_COMPONENT(MeshRenderer);
    public:
        MeshRenderer(SharedMesh &&mesh, std::string path);
        ~MeshRenderer() override = default;

        void addUMaterial(std::shared_ptr<UberMaterial> material);
        void addUMaterial(const std::shared_ptr<UberMaterial>&material, int index);

    protected:
        void onDrawUi() override;
        void onPreRender() override;
        void drawMeshOptions();
        void drawMaterialArray();

        void drawMaterialElementColumn(const std::string& name, int index);

        bool drawMaterialElement(int index);

        // An ideal world with have all of these with an equal length. Some mesh renderers can have multiple meshes
        // but only a single material.
        SharedMesh mMeshes;
        std::vector<std::shared_ptr<UberMaterial>> mUberMaterials;
        
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
