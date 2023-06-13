/**
 * @file Timers.h
 * @author Ryan Purse
 * @date 14/02/2022
 */


#pragma once

#include "Pch.h"
#include <glfw3.h>

namespace timers
{
    extern double deltaTime_impl;
    extern double fixedTime_impl;
    
    void update();
    
    template<typename T>
    T getTicks()
    {
        return static_cast<T>(glfwGetTime());
    }
    
    template<typename T>
    T deltaTime()
    {
        return static_cast<T>(deltaTime_impl);
    }
    
    template<typename T>
    constexpr T fixedTime()
    {
        return static_cast<T>(fixedTime_impl);
    }
    
}