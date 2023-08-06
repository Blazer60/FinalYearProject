/**
 * @file EngineState.h
 * @author Ryan Purse
 * @date 06/08/2023
 */


#pragma once

#include "Pch.h"
#include "Logger.h"

namespace engine
{
    extern Logger *logger;
    
    void forwardOpenGlCallback(
        GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length, const GLchar *message,
        const void *userParam);
}
