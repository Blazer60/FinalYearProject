#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 u_mvp_matrix;

void main()
{
    gl_Position = u_mvp_matrix * vec4(position.xyz, 1.f);
}