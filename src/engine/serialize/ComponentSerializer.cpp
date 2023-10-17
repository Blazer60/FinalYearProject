/**
 * @file ComponentSerializer.cpp
 * @author Ryan Purse
 * @date 15/10/2023
 */


#include "ComponentSerializer.h"
#include <Statistics.h>
#include "MeshComponent.h"
#include "yaml-cpp/emitter.h"
#include "EngineState.h"

namespace engine
{
    void attachComponentSerialization()
    {
        serializer->pushComponentDelegate([](YAML::Emitter &out, Component *component) -> bool {
            if (auto meshComponent = dynamic_cast<MeshComponent*>(component); meshComponent != nullptr)
            {
                serializeComponent(out, meshComponent);
                return true;
            }
            return false;
        });
    }
}

void serializeComponent(YAML::Emitter &out, engine::MeshComponent *meshComponent)
{
    out << YAML::Key << "Component" << YAML::Value << "MeshComponent";
    out << YAML::Key << "show" << YAML::Value << meshComponent->mShow;
}
