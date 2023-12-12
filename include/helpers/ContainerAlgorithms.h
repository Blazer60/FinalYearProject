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
}
