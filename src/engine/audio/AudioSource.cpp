/**
 * @file AudioSource.cpp
 * @author Ryan Purse
 * @date 03/12/2023
 */


#include "AudioSource.h"

namespace engine
{
    AudioSource::AudioSource(const std::filesystem::path& path)
        : mBuffer(path)
    {
        alGenSources(1, &mId);
        alSourcei(mId, AL_BUFFER, mBuffer.id());
    }

    AudioSource::~AudioSource()
    {
        if (mId != 0)
            alDeleteSources(1, &mId);
    }

    void AudioSource::play() const
    {
        alSourcePlay(mId);
    }
}
