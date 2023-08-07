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

namespace engine
{

/**
 * @author Ryan Purse
 * @date 07/08/2023
 */
    class MeshComponent
        : public Component
    {
    public:
        MeshComponent(SharedMesh sharedMesh, SharedMaterials sharedMaterials);
        
    protected:
        void onUpdate() override;
        void onDrawUi() override;
    
    protected:
        SharedMesh      mSharedMesh;
        SharedMaterials mSharedMaterials;
    };
    
} // engine
