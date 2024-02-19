#version 460 core

const float PI = 3.14159265359f;

vec3 fresnelSchlick(vec3 f0, float nDotL)
{
    return f0 + (1.f - f0) * pow(1 - nDotL, 5);
}
