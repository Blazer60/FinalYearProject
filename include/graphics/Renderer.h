/**
 * @file Renderer.h
 * @author Ryan Purse
 * @date 13/06/2023
 */


#pragma once

#include "Pch.h"
#include <functional>

namespace renderer
{
    bool init();
    
    /**
     * @returns false if debug message was failed to be setup. This is most likely due to the openGl version being
     * less than 4.3.
     * @see https://docs.gl/gl4/glDebugMessageCallback
     */
    bool debugMessageCallback(GLDEBUGPROC callback);
    
    std::string getVersion();
};
