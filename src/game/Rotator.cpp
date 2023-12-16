/**
 * @file Rotator.cpp
 * @author Ryan Purse
 * @date 31/08/2023
 */


#include "Rotator.h"
#include <Engine.h>

Rotator::Rotator(const glm::vec3 &rotation)
    : mRotation(rotation)
{

}

void Rotator::onUpdate()
{
    mActor->rotation = mActor->rotation * engine::quat(mRotation * timers::deltaTime<float>());
}

void Rotator::onDrawUi()
{
    engine::ui::PushID("Rotator");
    if (engine::ui::TreeNodeEx("Rotator Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (engine::ui::Button("Destroy Component"))
            mActor->removeComponent(this);
        
        engine::ui::DragFloat3("Angle Speed", engine::value_ptr(mRotation), 0.01f);
        
        engine::ui::TreePop();
    }
    
    engine::ui::PopID();
}

void serializeComponent(engine::serialize::Emitter &out, Rotator *rotator)
{
    out << engine::serialize::Key << "Component" << engine::serialize::Value << "Rotator";
    out << engine::serialize::Key << "Rotation" << engine::serialize::Value << rotator->mRotation;
}
