/**
 * @file Timers.h
 * @author Ryan Purse
 * @date 14/02/2022
 */


#pragma once

#include "Pch.h"

namespace timers
{
    extern double deltaTime_impl;
    extern double fixedTime_impl;
    
    void update();
    
    template<typename T>
    T getTicks()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        const long long nowNanoSeconds = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
        const double nowSeconds = static_cast<double>(nowNanoSeconds) * 1e-9;
        return static_cast<T>(nowSeconds);
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