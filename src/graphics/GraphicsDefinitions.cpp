/**
 * @file RendererHelpers.cpp
 * @author Ryan Purse
 * @date 27/06/2023
 */


#include "GraphicsDefinitions.h"

#include "DebugGBufferBlock.h"

namespace graphics
{
    constexpr GLint filterToGLenum[] { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
    constexpr GLint filterMagToGLenum[] { GL_NEAREST, GL_LINEAR, GL_NEAREST, GL_NEAREST, GL_LINEAR, GL_LINEAR };
    constexpr GLint wrapToGLenum[] { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE };

    constexpr int gbufferToInt[]
    {
        DEBUG_GBUFFER_NORMAL, DEBUG_GBUFFER_ROUGHNESS, DEBUG_GBUFFER_DIFFUSE,
        DEBUG_GBUFFER_SPECULAR, DEBUG_GBUFFER_EMISSIVE, DEBUG_GBUFFER_BYTE_COUNT,
        DEBUG_GBUFFER_FUZZ_COLOUR, DEBUG_GBUFFER_FUZZ_ROUGHNESS
    };

    constexpr GLenum formatToEnum[] { GL_RGBA16F, GL_RG16F, GL_R16F, GL_DEPTH_COMPONENT32F, GL_RGBA16, GL_RGBA32UI };
    constexpr GLenum pixelFormatToEnum[] { GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT, GL_STENCIL_INDEX };

    GLint toGLint(filter f)
    {
        return filterToGLenum[static_cast<int>(f)];
    }
    
    GLint toGLint(wrap w)
    {
        return wrapToGLenum[static_cast<int>(w)];
    }

    GLenum toGLenum(textureFormat f)
    {
        return formatToEnum[static_cast<int>(f)];
    }

    GLenum toGLenum(pixelFormat p)
    {
        return pixelFormatToEnum[static_cast<int>(p)];
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
