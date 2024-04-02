/**
 * @file LookAtActor.cpp
 * @author Ryan Purse
 * @date 01/04/2024
 */


#include "LookAtActor.h"
#include <Engine.h>

void LookAtActor::onBegin()
{
    if (trackedActorId != 0)
        trackedActor = mActor->getScene()->getActor(trackedActorId);
}

void LookAtActor::onUpdate()
{
    if (!trackedActor.isValid())
        return;

    const float direction = invertZ ? -1.f : 1.f;
    const glm::quat rotation = glm::quatLookAt(glm::normalize(direction * (trackedActor->getWorldPosition() - mActor->getWorldPosition())), glm::vec3(0.f, 1.f, 0.f));
    mActor->setWorldRotation(rotation);
}

void LookAtActor::onDrawUi()
{
    engine::ui::PushID("LookAtActor");
    if (engine::ui::TreeNodeEx("LookAtActor Settings", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth))
    {
        if (engine::ui::Button("Destroy Component"))
        {
            mActor->removeComponent(this);
        }

        const std::string_view actorName = trackedActor.isValid() ? trackedActor->getName() : "None";
        ImGui::Text("%s", actorName.data());
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ActorHierarchy"))
            {
                if (const Ref<engine::Actor> payloadActor = *static_cast<Ref<engine::Actor>*>(payload->Data); payloadActor.isValid())
                {
                    if (payloadActor.get() != mActor)
                    {
                        trackedActor = payloadActor;
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
        engine::ui::Checkbox("Invert Z?", &invertZ);

        engine::ui::TreePop();
    }

    engine::ui::PopID();
}

void serializeComponent(YAML::Emitter &out, LookAtActor *lookAtActor)
{
    out << engine::serialize::Key << "Component" << engine::serialize::Value << "LookAtActor";
    out << engine::serialize::Key << "ActorId" << engine::serialize::Value;
    if (lookAtActor->trackedActor.isValid())
        out << lookAtActor->trackedActor->getId();
    else
        out << 0;
    out << engine::serialize::Key << "InvertZ" << engine::serialize::Value << lookAtActor->invertZ;
}

