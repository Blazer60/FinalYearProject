/**
 * @file EngineRandom.h
 * @author Ryan Purse
 * @date 02/12/2023
 */


#pragma once

#include "Pch.h"

namespace engine
{
    typedef uint64_t UUID;
}

namespace engine::random
{
    UUID generateId();
}