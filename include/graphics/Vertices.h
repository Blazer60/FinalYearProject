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

struct PositionVertex
    : public Vertex
{
    constexpr explicit PositionVertex(glm::vec3 position)
        : position(position) {}
        
    constexpr explicit PositionVertex(const ObjVertex &objVertex)
        : position(objVertex.position) {}
    
    glm::vec3 position;
    
    static Instructions layout()
    {
        return
        {
            { GL_FLOAT, 3 }
        };
    };
};