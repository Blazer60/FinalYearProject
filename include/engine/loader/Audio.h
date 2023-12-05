/**
 * @file Audio.h
 * @author Ryan Purse
 * @date 03/12/2023
 */


#pragma once

#include <filesystem>

#include "../audio/AudioSource.h"
#include "../../Pch.h"

namespace load
{
    std::unique_ptr<engine::AudioSource> audio(const std::filesystem::path &path);
}