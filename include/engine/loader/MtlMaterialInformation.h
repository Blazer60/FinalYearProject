/**
 * @file MtlMaterialInformation.h
 * @author Ryan Purse
 * @date 20/06/2023
 */


#pragma once

#include "Pch.h"

#include <glm.hpp>
#include <string>


/**
 * @author Ryan Purse
 * @date 20/06/2023
 */
struct MtlMaterialInformation
{
    unsigned int    nS      { 0 };
    glm::vec3       kA      { 0.f };
    glm::vec3       kD      { 0.f };
    glm::vec3       kS      { 0.f };
    unsigned int    d       { 1 };
    unsigned int    illum   { 2 };
    std::string     mapKd   { "" };
    std::string     mapKe   { "" };
    std::string     mapNs   { "" };
};
