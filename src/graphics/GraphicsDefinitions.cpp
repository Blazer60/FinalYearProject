/**
 * @file RendererHelpers.cpp
 * @author Ryan Purse
 * @date 27/06/2023
 */


#include "GraphicsDefinitions.h"

#include "DebugGBufferBlock.h"

namespace graphics
{
    GLint filterToGLenum[] { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
    GLint filterMagToGLenum[] { GL_NEAREST, GL_LINEAR, GL_NEAREST, GL_NEAREST, GL_LINEAR, GL_LINEAR };
    GLint wrapToGLenum[] { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE };
    int gbufferToInt[] { DEBUG_GBUFFER_NORMAL, DEBUG_GBUFFER_ROUGHNESS, DEBUG_GBUFFER_DIFFUSE, DEBUG_GBUFFER_SPECULAR, DEBUG_GBUFFER_EMISSIVE, DEBUG_GBUFFER_BYTE_COUNT };
    
    GLint toGLint(filter f)
    {
        return filterToGLenum[static_cast<int>(f)];
    }
    
    GLint toGLint(wrap w)
    {
        return wrapToGLenum[static_cast<int>(w)];
    }

    GLint toMagGLint(filter f)
    {
        return filterMagToGLenum[static_cast<int>(f)];
    }

    int toInt(gbuffer g)
    {
        return gbufferToInt[static_cast<int>(g)];
    }

}
