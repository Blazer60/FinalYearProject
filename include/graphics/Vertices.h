/**
 * @file Vertices.h
 * @author Ryan Purse
 * @date 15/06/2023
 */


#pragma once

#include "Pch.h"
#include <Statistics.h>

/**
 * Contains a position, uv coords and normal. Loaded in from .obj files.
 * Define a conversion constructor to transition from this type.
 */
struct ObjVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
};

/**
 * @brief Loaded objects have these values set. Define a conversion constructor to transition
 * from this type.
 */
struct AssimpVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
};

/**
 * @brief An instruction that defines how a single element should be setup with the vao.
 */
struct Instruction
{
    GLenum type;
    int count;
};

typedef std::vector<Instruction> Instructions;

/**
 * @brief A base vertex to inherit off of.
 */
struct Vertex {};

/**
 * @brief A vertex that only contains a position and uv coordinate. Used to render fullscreen triangles to the screen.
 */
struct BasicVertex
    : public Vertex
{
    constexpr explicit BasicVertex(const glm::vec3 &position, const glm::vec2 &uv)
        : position(position), uv(uv) {}
        
    constexpr explicit BasicVertex(const ObjVertex &objVertex)
        : position(objVertex.position), uv(objVertex.uv) {}
    
    glm::vec3 position;
    glm::vec2 uv;
    
    static Instructions layout()
    {
        return
        {
            { GL_FLOAT, 3 },
            { GL_FLOAT, 2 }
        };
    };
};

struct StandardVertex
    : public Vertex
{
    constexpr explicit StandardVertex(const ObjVertex &objVertex)
        : position(objVertex.position), uv(objVertex.uv), normal(objVertex.normal), tangent(objVertex.tangent)
    {
    
    }
    
    constexpr explicit StandardVertex(const AssimpVertex &v)
        : position(v.position), uv(v.uv), normal(v.normal), tangent(v.tangent)
    {
    
    }
    
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    
    static Instructions layout()
    {
        return
            {
                { GL_FLOAT, 3 },
                { GL_FLOAT, 2 },
                { GL_FLOAT, 3 },
                { GL_FLOAT, 3 },
            };
    };
};

/**
 * @brief A Vertex that only holds a position. Point lights use these as everything else come from uniforms.
 * We just need to know where to place the light.
 */
struct PositionVertex
    : public Vertex
{
    constexpr explicit PositionVertex(const ObjVertex &objVertex)
        : position(objVertex.position)
    {
    }
    
    constexpr explicit PositionVertex(const AssimpVertex &v)
        : position(v.position)
    {
    }
    
    glm::vec3 position;
    
    static Instructions layout()
    {
        return { { GL_FLOAT, 3 } };
    }
};
