/**
 * @file Callback.h
 * @author Ryan Purse
 * @date 09/08/2023
 */


#pragma once

#include <functional>
#include "Pch.h"

template<typename ...TArgs>
class Callback
{
public:
    using CallbackFunc = std::function<void(const TArgs &...)>;
    
    uint32_t subscribe(const CallbackFunc & callback);
    void unSubscribe(uint32_t token);
    void broadcast(const TArgs &... args);
    
protected:
    std::vector<CallbackFunc> mSubscribers;
    std::vector<uint32_t> mTokens;
    
    uint32_t mNextTokenId { 0 };
};

template<typename... TArgs>
void Callback<TArgs...>::broadcast(const TArgs &... args)
{
    for (const CallbackFunc &subscriber : mSubscribers)
        subscriber(args...);
}

template<typename... TArgs>
void Callback<TArgs...>::unSubscribe(uint32_t token)
{
    auto it = std::find(mTokens.begin(), mTokens.end(), token);
    if (it != mTokens.end())
    {
        auto index = std::distance(mTokens.begin(), it);
        mSubscribers.erase(mSubscribers.begin() + index);
        mTokens.erase(it);
    }
}

template<typename... TArgs>
uint32_t Callback<TArgs...>::subscribe(const Callback::CallbackFunc & callback)
{
    mSubscribers.push_back(callback);
    mTokens.emplace_back(++mNextTokenId);
    return mNextTokenId;
}
