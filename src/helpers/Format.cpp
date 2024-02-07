/**
 * @file Format.cpp
 * @author Ryan Purse
 * @date 10/10/2023
 */


#include "Format.h"

#include <sstream>


namespace format
{
    template<>
    std::string value(const std::filesystem::path &value)
    {
        return value.string();
    }

    template<>
    std::string value<std::stringstream>(const std::stringstream& value)
    {
        return value.str();
    }

    template<>
    std::string value(const bool &value)
    {
        return value ? "true" : "false";
    }
}
