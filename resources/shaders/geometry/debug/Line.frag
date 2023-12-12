#version 460

layout(location = 0) out vec4 o_colour;

uniform vec3 u_colour;

void main()
{
    o_colour = vec4(u_colour, 1.f);
}
