/**
 * @file ModelLoader.cpp
 * @author Ryan Purse
 * @date 12/03/2022
 */


#include "ModelLoader.h"
#include "CommonLoader.h"

namespace load
{
    ObjVertex ObjectFile::createVertex(std::string_view arg)
    {
        const std::vector<std::string> nums = split(arg, '/');
        uint32_t positionIndex  { 0 };
        uint32_t uvIndex        { 0 };
        uint32_t normalIndex    { 0 };
        
        switch (nums.size())
        {
            case 3:  // Deliberately cascades down.
                normalIndex = std::stoi(nums[2]);
            case 2:
                if (!nums[1].empty())
                    uvIndex = std::stoi(nums[1]);
            case 1:
            default:
                positionIndex = std::stoi(nums[0]);
        }
        
        // .obj files start counting at one, so we can use it as a sanity check.
        return ObjVertex {
            positionIndex   == 0 ? glm::vec3(0.f) : positions[positionIndex - 1],
            uvIndex         == 0 ? glm::vec2(0.f) : uvs[uvIndex - 1],
            normalIndex     == 0 ? glm::vec3(0.f) : normals[normalIndex - 1],
        };
    }
}


