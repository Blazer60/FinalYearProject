/**
 * @file LtcSheenTable.h
 * @author Ryan Purse
 * @date 04/03/2024
 */


#pragma once

#include <array>

#include "Pch.h"

namespace graphics::sheen
{
    constexpr uint32_t tableSize = 32;
    constexpr uint32_t floatCount = 3 * tableSize * tableSize;
    std::array<float, floatCount> data();
    std::array<std::array<glm::vec3, tableSize>, tableSize> table();
}
