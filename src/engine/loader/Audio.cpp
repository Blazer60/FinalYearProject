/**
 * @file Audio.cpp
 * @author Ryan Purse
 * @date 03/12/2023
 */


#include "../../../include/engine/loader/Audio.h"

#include "EngineState.h"
#include "ResourcePool.h"

namespace load
{
    engine::AudioSource audio(const std::filesystem::path& path)
    {
        return engine::AudioSource(engine::resourcePool->loadAudioBuffer(path));
    }
}
