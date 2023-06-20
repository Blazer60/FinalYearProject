/**
 * @file Vertices.h
 * @author Ryan Purse
 * @date 15/06/2023
 */


#pragma once

#include "Pch.h"

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
    explicit PositionVertex(glm::vec3 position)
        : position(position) {}
    
    glm::vec3 position;
    
    static Instructions layout()
    {
        return
        {
            { GL_FLOAT, 3 }
        };
    };
};
