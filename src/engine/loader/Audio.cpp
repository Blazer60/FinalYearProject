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
    std::unique_ptr<engine::AudioSource> audio(const std::filesystem::path& path)
    {
        return std::make_unique<engine::AudioSource>(engine::resourcePool->loadAudioBuffer(path));
    }
}
