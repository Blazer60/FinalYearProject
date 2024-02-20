#version 460 core

vec3 linearToSRgb(vec3 linear)
{
    return pow(linear, vec3(0.45454545));
}
