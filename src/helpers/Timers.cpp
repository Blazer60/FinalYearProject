/**
 * @file Timers.cpp
 * @author Ryan Purse
 * @date 14/02/2022
 */


#include "Timers.h"

namespace timers
{
    double deltaTime_impl    { 0.16f };
    double fixedTime_impl    { 0.01f };  // 100 Ticks per second.
    
    static long long current           { 0 };
    static long long last              { 0 };
    
    void update()
    {
        auto now = std::chrono::high_resolution_clock::now();
        current = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
        deltaTime_impl = static_cast<double>(current - last) * 1e-9;
        last = current;
    }
}