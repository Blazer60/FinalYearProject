/**
 * @file Renderer.cpp
 * @author Ryan Purse
 * @date 13/06/2023
 */


#include "Renderer.h"

bool renderer::init()
{
    if (glewInit() != GLEW_OK)
        return false;
    
    // Blending texture data / enabling lerping.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    return true;
}

bool renderer::debugMessageCallback(GLDEBUGPROC callback)
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(callback, nullptr);
    
    int flags { 0 };  // Check to see if OpenGL debug context was set up correctly.
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    
    return (flags & GL_CONTEXT_FLAG_DEBUG_BIT);
}

std::string renderer::getVersion()
{
    return (reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}
