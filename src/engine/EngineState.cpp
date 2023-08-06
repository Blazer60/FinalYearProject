/**
 * @file EngineState.cpp
 * @author Ryan Purse
 * @date 06/08/2023
 */


#include "EngineState.h"

namespace engine
{
    Logger *logger;
    
    void forwardOpenGlCallback(
        GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
        const void *userParam)
    {
        logger->openglCallBack(source, type, id, severity, length, message, userParam);
    }
}
