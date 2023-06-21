/**
 * @file SubMesh.h
 * @author Ryan Purse
 * @date 15/06/2023
 */


#pragma once

#include "Pch.h"
#include "Vertices.h"

class SubMesh;

typedef std::vector<std::shared_ptr<SubMesh>> Mesh;

void setVaoLayout(unsigned int vao, const Instructions &instructions);

/**
 * @author Ryan Purse
 * @date 15/06/2023
 */
class SubMesh
{
public:
    template<typename TVertex>
    SubMesh(const std::vector<TVertex> &vertices, const std::vector<uint32_t> &indices);
    
    ~SubMesh();
    
    [[nodiscard]] uint32_t vao() const { return mVao; }
    [[nodiscard]] int32_t  indicesCount() const { return mIndicesCount; };
    
protected:
    uint32_t mVao { 0 };
    uint32_t mVbo { 0 };
    uint32_t mEbo { 0 };
    int32_t  mIndicesCount { 0 };
};

template<typename TVertex>
SubMesh::SubMesh(const std::vector<TVertex> &vertices, const std::vector<uint32_t> &indices)
{
    glCreateBuffers(1, &mVbo);
    glCreateBuffers(1, &mEbo);
    glNamedBufferData(mVbo, vertices.size() * sizeof(TVertex), static_cast<const void *>(&vertices[0]), GL_STATIC_DRAW);
    glNamedBufferData(mEbo, static_cast<int64_t>(indices.size() * sizeof(uint32_t)), static_cast<const void *>(&indices[0]), GL_STATIC_DRAW);
    mIndicesCount = static_cast<int32_t>(indices.size());
    
    glCreateVertexArrays(1, &mVao);
    
    const unsigned int bindingIndex = 0;
    const unsigned int offSet = 0;
    const unsigned int stride = sizeof(TVertex);
    
    setVaoLayout(mVao, TVertex::layout());
    
    glVertexArrayVertexBuffer(mVao, bindingIndex, mVbo, offSet, stride);
    glVertexArrayElementBuffer(mVao, mEbo);
}
