#version 460

layout(location = 0) in uint a_id;

uniform mat4 u_mvp_matrix;
uniform vec3 u_locationA;
uniform vec3 u_locationB;

void main()
{
    if (a_id == 0)
        gl_Position = u_mvp_matrix * vec4(u_locationA, 1.f);
    else
        gl_Position = u_mvp_matrix * vec4(u_locationB, 1.f);
}
