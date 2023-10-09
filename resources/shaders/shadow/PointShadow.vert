#version 460

layout(location=0) in vec3 a_position;

uniform mat4 u_model_matrix;
uniform mat4 u_mvp_matrix;

out vec3 v_position;

void main()
{
    v_position = (u_model_matrix * vec4(a_position, 1.f)).xyz;
    gl_Position = u_mvp_matrix * vec4(a_position, 1.f);
}
