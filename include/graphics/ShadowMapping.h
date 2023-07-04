/**
 * @file ShadowMapping.h
 * @author Ryan Purse
 * @date 03/07/2023
 */


#pragma once

#include "Pch.h"
#include "CameraSettings.h"

namespace renderer
{
    namespace shadow
    {
        extern float zMultiplier;
        extern std::vector<float> cascadeMultipliers;
        extern uint32_t cascadeZones;
        extern glm::vec2 bias;
    }
    
    void shadowMapping(const CameraSettings &cameraSettings, const std::vector<float> &cascadeDepths);
}
