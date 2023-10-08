/**
 * @file Action.h
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
 * @author Ryan Purse
 * @date 23/08/2023
 */
template<typename ...TArgs>
class Input
    : public Callback<TArgs...>
{
    virtual void doAction() = 0;
};

class Button
    : public Input<>
{
public:
    explicit Button(ImGuiKey key);
    void doAction() override;
    
protected:
    std::vector<ImGuiKey> mKeys;
};

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

class ScrollWheel
    : public Input<float>
{
public:
    void doAction() override;
};
