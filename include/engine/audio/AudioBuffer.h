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

        [[nodiscard]] ALuint id() const;
        [[nodiscard]] std::filesystem::path getPath() const;
    protected:
        ALuint mId { 0 };
        std::filesystem::path mPath;
    };
} // engine
