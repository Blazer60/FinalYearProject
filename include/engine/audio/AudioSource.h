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
        explicit AudioSource(const std::filesystem::path &path);
        ~AudioSource();

        void play() const;

    protected:
        ALuint mId { 0 };
        AudioBuffer mBuffer;
    };

} // engine
