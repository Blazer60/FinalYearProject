#version 460 core

layout(location = 0) out vec3 o_colour;

uniform vec3 u_colour;

void main()
{
    o_colour.rgb = u_colour;
}