/**
 * @file Spammer.cpp
 * @author Ryan Purse
 * @date 28/10/2023
 */


#include "Spammer.h"
#include <Engine.h>

void Spammer::onBegin()
{
    for (int i = 0; i < 30; ++i)
    {
        const Ref<engine::Actor> rootActor = engine::core->getScene()->spawnActor<engine::Actor>("E" + std::to_string(i));
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
    mMeshRenderer->addUMaterial(std::make_shared<engine::UberMaterial>(""));
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
    engine::ui::PushID("Spammer Settings");
    if (engine::ui::TreeNodeEx("Spammer Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (engine::ui::Button("Destroy Component"))
            mActor->removeComponent(this);
        
        engine::ui::TreePop();
    }
    engine::ui::PopID();
}


