/**
 * @file Spammer.cpp
 * @author Ryan Purse
 * @date 28/10/2023
 */


#include "Spammer.h"
#include "MeshRenderer.h"
#include <Statistics.h>
#include <EngineState.h>
#include <Core.h>
#include <Scene.h>
#include <MaterialSubComponent.h>

void Spammer::onBegin()
{
    for (int i = 0; i < 30; ++i)
    {
        Ref<engine::Actor> rootActor = engine::core->getScene()->spawnActor<engine::Actor>("E" + std::to_string(i));
        mActors.push_back(rootActor);
        
        auto childActor = getActor()->addChildActor(makeResource<engine::Actor>("F" + std::to_string(i)));
        mActors.push_back(childActor);
    }
    
    mActor->addComponent(load::meshRenderer<StandardVertex>(""));  // This does return a ref<> but we assume are user doesn't know that.
    
    // Should be guaranteed since we've just created it with the line above.
    if (mActor->hasComponent<engine::MeshRenderer>())
        mMeshRenderer = getActor()->getComponent<engine::MeshRenderer>();
    
    mActor->removeComponent<engine::MeshRenderer>();
    // mMeshRenderer is now a dangling reference.
    
    // Should error here.
    WARN("The next error should occur.");
    mMeshRenderer->addMaterial(std::make_shared<engine::StandardMaterialSubComponent>());
}

void Spammer::onUpdate()
{

}

Spammer::~Spammer()
{
    for (auto &item : mActors)
        item->markForDeath();
    mActors.clear();
    engine::core->getScene()->spawnActor<engine::Actor>("Virus Remains");
    
    // mActor->removeComponent(this);
    mActor->removeComponent(mMeshRenderer);
}

void Spammer::onDrawUi()
{
    ImGui::PushID("Spammer Settings");
    if (ImGui::TreeNodeEx("Spammer Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Destroy Component"))
            mActor->removeComponent(this);
        
        ImGui::TreePop();
    }
    ImGui::PopID();
}


