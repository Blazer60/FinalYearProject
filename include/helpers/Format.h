/**
 * @file Format.h
 * @author Ryan Purse
 * @date 10/10/2023
 */


#pragma once

#include "Pch.h"
#include <filesystem>

namespace format
{
    enum format_type_
    {
        format_type_default,
        format_type_debug,
    };
    typedef format_type_ format_type;
    
    constexpr bool operator==(format_type lhs, format_type rhs)
    {
        return static_cast<int>(lhs) == static_cast<int>(rhs);
    }
    
    template<typename T>
    std::string value(const T &value);
    
    template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
    std::string value(glm::vec<L, T, Q> vector);

    template<typename T, glm::qualifier Q = glm::defaultp>
    std::string value(glm::qua<T, Q> quaternion, format_type type=format_type_default);

    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q = glm::defaultp>
    std::string value(glm::mat<C, R, T, Q> matrix, format_type type=format_type_default);

    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q = glm::defaultp>
    std::string debug(glm::mat<C, R, T, Q> matrix);

    template<>
    std::string value(const std::filesystem::path &value);

    template<>
    std::string value(const bool &value);
    
    template<typename TIterator>
    std::string value(const TIterator &start, const TIterator &end);

    template<typename TIterator, typename TPred>
    std::string value(const TIterator &start, const TIterator &end, TPred pred);
    
    template<typename T>
    std::string type(const T &value);
    
    template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
    std::string type(glm::vec<L, T, Q> vector);
    
    template<typename T, glm::qualifier Q = glm::defaultp>
    std::string type(glm::qua<T, Q> quaternion);
    
    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q = glm::defaultp>
    std::string type(glm::mat<C, R, T, Q> matrix);
    
    
    
    // Definitions
    
    template<typename T>
    std::string value(const T &value)
    {
        if constexpr (std::is_arithmetic_v<T>)
            return std::to_string(value);
        else
            return std::string(value);
    }
    
    
    template<glm::length_t L, typename T, glm::qualifier Q>
    std::string value(glm::vec<L, T, Q> vector)
    {
        std::string output = "(";

        for (glm::length_t i = 0; i < L - 1; ++i)
            output += value(vector[i]) + ", ";

        return output + value(vector[L - 1]) + ")";
    }

    template<typename T, glm::qualifier Q>
    std::string value(glm::qua<T, Q> quaternion, format_type type)
    {
        std::string output = "(";

        for (int i = 0; i < 3; ++i)
            output += value(quaternion[i]) + ", ";

        output += value(quaternion[3]) + ")";
        if (type == format_type_debug)
            return output + " Euler" + value(glm::eulerAngles(quaternion));
        return output;
    }

    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
    std::string value(glm::mat<C, R, T, Q> matrix, format_type type)
    {
        if (type == format_type_debug)
            return debug(matrix);

        std::string output;
        for (int i = 0; i < C; ++i)
            output += value(glm::row(matrix, i)) + ", ";

        return output;
    }

    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
    std::string debug(glm::mat<C, R, T, Q> matrix)
    {
        std::string output = "\n";

        for (int i = 0; i < C; ++i)
            output += value(glm::row(matrix, i)) + "\n";

        return output;
    }

    template<typename TIterator>
    std::string value(const TIterator &start, const TIterator &end)
    {
        return value(start, end, [](const auto &elem){ return value(elem); });
    }

    template<typename TIterator, typename TPred>
    std::string value(const TIterator& start, const TIterator& end, TPred pred)
    {
#ifdef _MSC_VER
        std::_Adl_verify_range(start, end);
#endif

        std::string output = "[";
        TIterator element = start;
        while (true)
        {
            output += pred(*element);
            ++element;
            if (element != end)
                output += ", ";
            else
                break;
        }

        output += "]";
        return output;
    }

    template<typename T>
    std::string type(const T &value)
    {
        if constexpr (std::is_arithmetic_v<T>)
            return "num";
        else
            return "str";
    }
    
    template<glm::length_t L, typename T, glm::qualifier Q>
    std::string type(glm::vec<L, T, Q> vector)
    {
        return "vec" + std::to_string(L);
    }
    
    template<typename T, glm::qualifier Q>
    std::string type(glm::qua<T, Q> quaternion)
    {
        return "quat";
    }
    
    template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
    std::string type(glm::mat<C, R, T, Q> matrix)
    {
        return "mat" + std::to_string(C) + "x" + std::to_string(R);
    }
}
