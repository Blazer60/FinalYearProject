/**
 * @file MeshComponent.h
 * @author Ryan Purse
 * @date 07/08/2023
 */


#pragma once

#include "Pch.h"
#include "Component.h"
#include "Mesh.h"
#include "Materials.h"
#include "ComponentSerializer.h"

namespace engine
{
/**
 * @author Ryan Purse
 * @date 07/08/2023
 */
    class MeshComponent
        : public Component
    {
        SERIALIZABLE(MeshComponent);
    public:
        MeshComponent(SharedMesh sharedMesh, SharedMaterials sharedMaterials);
        
        template<typename TMaterial>
        MeshComponent(SharedMesh sharedMesh, std::shared_ptr<TMaterial> material);
        
    protected:
        void onDrawUi() override;
        void onPreRender() override;
        
        void drawMeshOptions();
    
    protected:
        SharedMesh      mSharedMesh;
        SharedMaterials mSharedMaterials;
        bool            mShow { true };
    };
    
    template<typename TMaterial>
    MeshComponent::MeshComponent(SharedMesh sharedMesh, std::shared_ptr<TMaterial> material)
        : mSharedMesh(std::move(sharedMesh))
    {
        mSharedMaterials.push_back(material);
    }
    
} // engine
