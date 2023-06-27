/**
 * @file RendererHelpers.cpp
 * @author Ryan Purse
 * @date 27/06/2023
 */


#include "RendererHelpers.h"

namespace renderer
{
    GLint filterToGLenum[] { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
    GLint wrapToGLenum[] { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE };
    
    GLint toGLint(Filter f)
    {
        return filterToGLenum[static_cast<int>(f)];
    }
    
    GLint toGLint(Wrap w)
    {
        return wrapToGLenum[static_cast<int>(w)];
    }
}
