/**
 * @file Input.h
 * @author Ryan Purse
 * @date 23/08/2023
 */


#pragma once

#include "Pch.h"
#include "imgui.h"
#include "Callback.h"

struct EventKeyValue
{
    ImGuiKey key;
    float value;
};

/**
 * A user input from keyboard or mouse that can be acted on.
 * @author Ryan Purse
 * @date 23/08/2023
 */
template<typename ...TArgs>
class Input
    : public Callback<TArgs...>
{
    virtual void doAction() = 0;
};

/**
 * \brief A keyboard input that has a binary state.
 * Multiple inputs can be registered to the same event.
 */
class Button
    : public Input<>
{
public:
    explicit Button(ImGuiKey key);
    void doAction() override;
    
protected:
    std::vector<ImGuiKey> mKeys;
};

/**
 * \brief An input that can be represent by a range of values between [0, 1].
 * Multiple inputs can be registered to the same event.
 */
class Value
    : public Input<float>
{
public:
    explicit Value(EventKeyValue keyValue);
    explicit Value(const std::vector<EventKeyValue> &keyValues);
    void doAction() override;
    
protected:
    std::vector<EventKeyValue> mKeyValues;
};

/**
 * \brief An input from the scroll-wheel. The speed of the scroll-wheel is broadcasted.
 */
class ScrollWheel
    : public Input<float>
{
public:
    void doAction() override;
};
