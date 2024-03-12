/**
 * @file ContainerAlgorithms.h
 * @author Ryan Purse
 * @date 12/12/2023
 */


#pragma once

#include <unordered_set>

namespace containers
{
    // Std::erase_if for unorded_set
    template<typename Key, typename Pr>
    size_t erase_if(std::unordered_set<Key> &c, const Pr &pred)
    {
        auto oldSize = c.size();
        for (auto first = c.begin(), last = c.end(); first != last;)
        {
            if (pred(*first))
                first = c.erase(first);
            else
                ++first;
        }

        return oldSize - c.size();
    }

    template<typename T, typename Tit>
    std::vector<T> makePattern(const Tit &begin, const Tit &end, uint32_t count)
    {
        std::vector<T> result;
        result.reserve(count * std::distance(begin, end));
        for (uint32_t i = 0; i < count; ++i)
            result.insert(result.end(), begin, end);

        return result;
    }

    template<typename T>
    void moveInPlace(std::vector<T> &vector, const int srcIndex, const int dstIndex)
    {
        if (srcIndex < dstIndex)
        {
            const auto lhs = vector.begin() + srcIndex;
            const auto pivot = lhs + 1;
            const auto rhs = vector.begin() + dstIndex + 1;
            std::rotate(lhs, pivot, rhs);
        }
        else
        {
            const auto lhs = vector.begin() + dstIndex;
            const auto pivot = vector.begin() + srcIndex;
            const auto rhs = pivot + 1;
            std::rotate(lhs, pivot, rhs);
        }
    }
}
