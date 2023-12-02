/**
 * @file EngineRandom.cpp
 * @author Ryan Purse
 * @date 02/12/2023
 */


#include "EngineRandom.h"

#include <random>

namespace engine
{
    std::random_device randomDevice;
    std::mt19937_64 gen64(randomDevice());
    std::uniform_int_distribution<UUID> uuidDistribution;

    UUID random::generateId()
    {
        return uuidDistribution(gen64);
    }
}
