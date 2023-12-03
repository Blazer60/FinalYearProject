/**
 * @file AudioBuffer.h
 * @author Ryan Purse
 * @date 03/12/2023
 */


#pragma once

#include <al.h>
#include <filesystem>

#include "Pch.h"

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 03/12/2023
     */
    class AudioBuffer
    {
    public:
        explicit AudioBuffer(const std::filesystem::path &path);
        ~AudioBuffer();

        ALuint id() const;
    protected:
        ALuint mId { 0 };
    };
} // engine
