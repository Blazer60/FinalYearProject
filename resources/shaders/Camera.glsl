#version 460 core

layout(std140) uniform CameraBlock
{
    vec3 position;
    mat4 viewMatrix;
} camera;
