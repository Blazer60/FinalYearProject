/**
 * @file ComponentSerializer.cpp
 * @author Ryan Purse
 * @date 15/10/2023
 */


#include "ComponentSerializer.h"
#include <Statistics.h>
#include "yaml-cpp/emitter.h"
#include "EngineState.h"

namespace engine
{
    void attachComponentSerialization()
    {
        // serializer->pushComponentDelegate([](YAML::Emitter &out, Component *component) -> bool {
        //     if (auto meshComponent = dynamic_cast<MeshComponent*>(component); meshComponent != nullptr)
        //     {
        //         CRASH("Not Implemented");
        //         // serializeComponent(out, meshComponent);
        //         return true;
        //     }
        //     return false;
        // });
    }
}
