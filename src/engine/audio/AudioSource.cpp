/**
 * @file AudioSource.cpp
 * @author Ryan Purse
 * @date 03/12/2023
 */


#include "AudioSource.h"

namespace engine
{
    AudioSource::AudioSource()
        : mBuffer(std::make_shared<AudioBuffer>(""))
    {
        alGenSources(1, &mId);
        alSourcei(mId, AL_BUFFER, static_cast<ALint>(mBuffer->id()));
    }

    AudioSource::AudioSource(std::shared_ptr<AudioBuffer> audioBuffer)
        : mBuffer(std::move(audioBuffer))
    {
        alGenSources(1, &mId);
        alSourcei(mId, AL_BUFFER, static_cast<ALint>(mBuffer->id()));
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

    std::filesystem::path AudioSource::getPath() const
    {
        if (mBuffer)
            return mBuffer->getPath();
        return "";
    }
}
