/**
 * @file StringManipulation.cpp
 * @author Ryan Purse
 * @date 23/02/2024
 */


#include "StringManipulation.h"

std::vector<std::string> split(const std::string& str, const char delim)
{
    std::vector<std::string> out;
    auto startIt = str.begin();
    auto endIt = startIt;

    const auto addIfNonZeroLength = [&] {
        if (startIt != endIt)
            out.emplace_back(startIt, endIt);
    };

    while (endIt != str.end())
    {
        if (*endIt == delim)
        {
            addIfNonZeroLength();
            startIt = std::next(endIt);
        }
        ++endIt;
    }

    addIfNonZeroLength();

    return out;
}

