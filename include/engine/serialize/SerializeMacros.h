/**
 * @file SerializeMacros.h
 * @author Ryan Purse
 * @date 14/10/2023
 */


#pragma once

#include "Pch.h"

#include "SceneSerializer.h"

namespace engine
{
    template<typename T>
    class SerializeComponent
    {
    public:
        explicit SerializeComponent()
        {
            serialize::pushComponentDelegate([](YAML::Emitter &out, Component *component) -> bool {
                if (auto x = dynamic_cast<T*>(component); x != nullptr)
                {
                    serializeComponent(out, x);
                    return true;
                }
                return false;
            });
        }
    };
}

#ifdef CONCAT
    #define CONCAT_INNER(x, y) x ## y
    #define CONCAT(x, y) CONCAT_INNER(x, y)
#endif

#define SERIALIZE_METHOD(class) engine::SerializeComponent<class> CONCAT(serializeMethodStaticInsert, __LINE__)
#define SERIALIZABLE(class) friend void serializeComponent(YAML::Emitter&, class*)
