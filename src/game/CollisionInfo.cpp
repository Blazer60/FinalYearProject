/**
 * @file CollisionInfo.cpp
 * @author Ryan Purse
 * @date 09/12/2023
 */


#include "CollisionInfo.h"
#include "Actor.h"
#include "SoundComponent.h"

void CollisionInfo::onBegin()
{
    mSound = mActor->getComponent<engine::SoundComponent>(false);
}

void CollisionInfo::onCollisionBegin(
 engine::Actor* otherActor, Component* myComponent, Component* otherComponent, const engine::HitInfo& hitInfo)
{
    MESSAGE("Collision Begin: %", hitInfo.normal);
    if (mSound.isValid())
    {
        mSound->playSound();
    }
}

void CollisionInfo::onDrawUi()
{
    ImGui::PushID("Collision Info");
    if (ImGui::TreeNodeEx("Collision Info Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Destroy Component"))
            mActor->removeComponent(this);

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void serializeComponent(YAML::Emitter &out, CollisionInfo *collisionInfo)
{
    out << YAML::Key << "Component" << YAML::Value << "CollisionInfo";
}

