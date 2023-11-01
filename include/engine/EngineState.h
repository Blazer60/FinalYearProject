/**
 * @file EngineState.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include "Pch.h"
#include "Logger.h"
#include "glew.h"

namespace engine
{
    extern class Core *core;
    extern class Editor *editor;
    extern class RootEventHandler *eventHandler;
    extern class Serializer *serializer;
    extern class ResourcePool *resourcePool;
    
    void GLAPIENTRY forwardOpenGlCallback(
        GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length, const GLchar *message,
        const void *userParam);
}
