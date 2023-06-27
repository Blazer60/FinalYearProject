#version 460

layout(location=0) in vec3 a_position;

uniform mat4 u_mvp_matrix;

void main()
{
    gl_Position = u_mvp_matrix * vec4(a_position, 1.f);
}