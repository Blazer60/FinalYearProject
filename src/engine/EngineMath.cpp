/**
 * @file Math.cpp
 * @author Ryan Purse
 * @date 08/08/2023
 */


#include "EngineMath.h"
#include "gtx/matrix_decompose.inl"
#include "imgui.h"
#include "ImGuizmo.h"

namespace math
{
    void decompose(const glm::mat4 &transform, glm::vec3 &position, glm::quat &rotation, glm::vec3 &scale)
    {
        glm::vec3 rot;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(position), glm::value_ptr(rot), glm::value_ptr(scale));
        rotation = glm::quat(glm::radians(rot));
    }

    float inverseLerp(const float from, const float to, const float value)
    {
        return (value - from) / (to - from);
    }

    glm::vec3 inverseLerp(const glm::vec3& from, const glm::vec3& to, const glm::vec3& value)
    {
        return (value - from) / (to - from);
    }
}
