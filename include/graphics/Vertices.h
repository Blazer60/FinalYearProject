/**
 * @file Vertices.h
 * @author Ryan Purse
 * @date 15/06/2023
 */


#pragma once

#include "Pch.h"

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
