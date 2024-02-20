#version 460 core

vec3 linearToSRgb(vec3 linear)
{
    return pow(linear, vec3(0.45454545));
}

vec3 sRgbToLinear(vec3 sRgb)
{
    const vec3 gamma = vec3(2.2f);
    return pow(sRgb, gamma);
}
