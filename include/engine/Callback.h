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
    virtual ~Callback() = default;

    /**
     * \brief Adds a delegate function that will be called from broadcast();
     * \param callback A delegate function
     * \returns A token used to unsubscribe the correct function.
     */
    uint32_t subscribe(const CallbackFunc & callback);

    /**
     * \brief Removes a delegate function.
     * \param token The token obtained from subscribe()
     */
    void unSubscribe(uint32_t token);

    /**
     * \brief Calls each delegate function in turn.
     * \param args The arguments that you want to be passed to each subscriber
     */
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
void Callback<TArgs...>::unSubscribe(const uint32_t token)
{
    if (token == 0)
        return;  // A token value of zero is always invalid.

    const auto it = std::find(mTokens.begin(), mTokens.end(), token);
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
