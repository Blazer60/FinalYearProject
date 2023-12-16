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
    mActor->rotation = mActor->rotation * glm::quat(mRotation * timers::deltaTime<float>());
}

void Rotator::onDrawUi()
{
    ImGui::PushID("Rotator");
    if (ImGui::TreeNodeEx("Rotator Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Destroy Component"))
            mActor->removeComponent(this);
        
        ImGui::DragFloat3("Angle Speed", glm::value_ptr(mRotation), 0.01f);
        
        ImGui::TreePop();
    }
    
    ImGui::PopID();
}

void serializeComponent(YAML::Emitter &out, Rotator *rotator)
{
    out << YAML::Key << "Component" << YAML::Value << "Rotator";
    out << YAML::Key << "Rotation" << YAML::Value << rotator->mRotation;
}
