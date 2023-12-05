/**
 * @file AudioSource.h
 * @author Ryan Purse
 * @date 03/12/2023
 */


#pragma once

#include <al.h>

#include "AudioBuffer.h"
#include "Pch.h"

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 03/12/2023
     */
    class AudioSource
    {
    public:
        AudioSource();
        AudioSource(AudioSource&) = delete;
        AudioSource operator=(AudioSource) = delete;
        explicit AudioSource(std::shared_ptr<engine::AudioBuffer> audioBuffer);

        ~AudioSource();

        void play() const;
        void setPosition(const glm::vec3 &position);
        void setVolume(float volume) const;
        std::filesystem::path getPath() const;

    protected:
        ALuint mId { 0 };
        std::shared_ptr<AudioBuffer> mBuffer;
        glm::vec3 mPosition { 0 };
    };

} // engine
