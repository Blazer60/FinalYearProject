/**
 * @file StringManipulation.h
 * @author Ryan Purse
 * @date 23/02/2024
 */


#pragma once

#include <sstream>

#include "Pch.h"

std::vector<std::string> split(const std::string &str, char delim=' ');

template<typename TIterator>
std::string strip(const TIterator &begin, const TIterator &end, const std::vector<char> &remove={ ' ' });

template<typename TIterator>
std::string join(const TIterator &begin, const TIterator &end, char delim=' ');

template<typename TIterator>
std::string strip(const TIterator &begin, const TIterator &end, const std::vector<char>& remove)
{
    std::stringstream out;
    auto it = begin;
    while (it != end)
    {
        if (std::find(remove.begin(), remove.end(), *it) == remove.end())
            out << *it;

        ++it;
    }

    return out.str();
}

template<typename TIterator>
std::string join(const TIterator& begin, const TIterator& end, const char delim)
{
    if (begin == end)
        return "";

    std::stringstream out;

    out << std::string(*begin);
    for (auto it = std::next(begin); it != end; ++it)
        out << delim << std::string(*it);

    return out.str();
}
