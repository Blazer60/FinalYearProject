/**
 * @file RawMesh.h
 * @author Ryan Purse
 * @date 13/03/2022
 */


#pragma once

#include "Pch.h"
#include "CommonLoader.h"
#include "Vertices.h"
#include "Mesh.h"

#include <functional>

namespace load
{
    template<typename TVertex>
    class RawMesh;
    
    /** A typedef short-hand for named vertex groups generated by an Object File. */
    template<typename TVertex>
    using RawMeshes = std::unordered_map<std::string, RawMesh<TVertex>>;
    
    typedef std::function<ObjVertex(std::string_view arg)> createObjectSignature;
    
    /**
     * @brief Contains vertex data that is generated from the Object file. Grouped by the material names.
     * @tparam TVertex - The type of vertex that you want to store.
     * @author Ryan Purse
     * @date 13/03/2022
     */
    template<typename TVertex>
    class RawMesh
    {
    public:
        void createVertices(std::string_view args, const createObjectSignature &createObjectVertex);
        void createIndices(const std::vector<uint32_t> &uniqueIndices);
        
        [[nodiscard]] std::shared_ptr<SubMesh> toSharedSubMesh() const;
        
        [[nodiscard]] uint32_t verticesSize() const { return mVertices.size() * sizeof(TVertex); }
        [[nodiscard]] uint32_t indicesSize() const { return mIndices.size() * sizeof(uint32_t); }
        [[nodiscard]] const void *verticesData() const { return static_cast<const void *>(&(mVertices.at(0))); }
        [[nodiscard]] const void *indicesData() const { return static_cast<const void *>(&(mIndices.at(0))); }
        [[nodiscard]] int indicesCount() const { return static_cast<int>(mIndices.size()); }
    
    protected:
        std::vector<uint32_t>                       mIndices;
        std::vector<TVertex>                        mVertices;
        
        void calculateTangents(std::vector<ObjVertex> &intermediateVertices);
    };
    
    template<typename TVertex>
    void RawMesh<TVertex>::calculateTangents(std::vector<ObjVertex> &intermediateVertices)
    {
        if (intermediateVertices.size() < 3)
        {
            WARN("The face does not contain at least three vertices. Tangent information cannot be generated. "
                 "Normal maps may not correct for this modelAndMaterial.");
            return;
        }
        // If a face has more than three vertices, we assume that the whole face is flat.
        
        const glm::vec3 &v0 = intermediateVertices[0].position;
        const glm::vec3 &v1 = intermediateVertices[1].position;
        const glm::vec3 &v2 = intermediateVertices[2].position;
        
        const glm::vec2 &uv0 = intermediateVertices[0].uv;
        const glm::vec2 &uv1 = intermediateVertices[1].uv;
        const glm::vec2 &uv2 = intermediateVertices[2].uv;
        
        const glm::vec3 deltaPos1 = v1 - v0;
        const glm::vec3 deltaPos2 = v2 - v0;
        
        const glm::vec2 deltaUv1 = uv1 - uv0;
        const glm::vec2 deltaUv2 = uv2 - uv0;
        
        const float r = 1.f / (deltaUv1.x * deltaUv2.y - deltaUv1.y * deltaUv2.x);
        const glm::vec3 tangent = (deltaPos1 * deltaUv2.y - deltaPos2 * deltaUv1.y) * r;
        
        for (auto &vertex : intermediateVertices)
            vertex.tangent = tangent;
    }
    
    template<typename TVertex>
    std::shared_ptr<SubMesh> RawMesh<TVertex>::toSharedSubMesh() const
    {
        return std::make_shared<SubMesh>(mVertices, mIndices);
    }
    
    template<typename TVertex>
    void RawMesh<TVertex>::createVertices(std::string_view args, const createObjectSignature &createObjectVertex)
    {
        std::vector<uint32_t> uniqueIndices;
        const std::vector<std::string> vertexIds = split(args);
        
        std::vector<ObjVertex> intermediateVertices;
        intermediateVertices.reserve(vertexIds.size());
        for (const std::string &id : vertexIds)
            intermediateVertices.emplace_back(createObjectVertex(id));
        
        calculateTangents(intermediateVertices);
        
        for (const ObjVertex &vertex : intermediateVertices)
        {
            mVertices.emplace_back(vertex);  // implicit conversion to user defined type.
            uniqueIndices.emplace_back(static_cast<uint32_t>((mVertices.size() - 1)));
        }
       
        createIndices(uniqueIndices);
    }
    
    template<typename TVertex>
    void RawMesh<TVertex>::createIndices(const std::vector<uint32_t> &uniqueIndices)
    {
        const uint32_t baseVertexIndex = uniqueIndices[0];
        // Indexes into the uniqueIndices.
        uint32_t firstIndex  = 1;
        uint32_t secondIndex = 2;
        // Create a fan out of the indices.
        for (int i = 0; i < uniqueIndices.size() - 2; ++i)
        {
            mIndices.emplace_back(baseVertexIndex);
            mIndices.emplace_back(uniqueIndices[firstIndex++]);
            mIndices.emplace_back(uniqueIndices[secondIndex++]);
        }
    }
}




