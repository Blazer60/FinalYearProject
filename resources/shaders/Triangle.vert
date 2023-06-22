#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 a_uv;

out vec2 v_uv;

uniform mat4 u_mvp_matrix;

void main()
{
    gl_Position = u_mvp_matrix * vec4(position.xyz, 1.f);
    v_uv = a_uv;
}