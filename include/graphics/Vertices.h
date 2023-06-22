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
        : position(position), uv(glm::vec2(0.f)) {}
        
    constexpr explicit PositionVertex(const ObjVertex &objVertex)
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
        : position(objVertex.position), uv(objVertex.uv), normal(objVertex.normal)
    {
    
    }
    
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    
    static Instructions layout()
    {
        return
            {
                { GL_FLOAT, 3 },
                { GL_FLOAT, 2 },
                { GL_FLOAT, 3 },
            };
    };
};
