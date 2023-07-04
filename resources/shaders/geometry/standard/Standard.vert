#version 460

layout(location=0) in vec3 a_position;
layout(location=1) in vec2 a_uv;
layout(location=2) in vec3 a_normal;
layout(location=3) in vec3 a_tangent;

uniform mat4 u_mvp_matrix;
uniform mat4 u_model_matrix;

out vec2 v_uv;
out vec3 v_position_ws;
out vec3 v_normal_ws;
out mat3 v_tbn_matrix;

void main()
{
    gl_Position = u_mvp_matrix * vec4(a_position, 1.f);
    v_uv = a_uv;
    v_normal_ws = vec3(u_model_matrix * vec4(a_normal, 0.f));
    v_position_ws = vec3(u_model_matrix * vec4(a_position.xyz, 1.f));

    vec3 tangent_ws = normalize(vec3(u_model_matrix * vec4(a_tangent, 0.f)));
    vec3 normal_ws = normalize(vec3(u_model_matrix * vec4(a_normal, 0.f)));
    vec3 bi_tangent_ws = cross(tangent_ws, normal_ws);
    v_tbn_matrix = transpose(mat3(tangent_ws, bi_tangent_ws, normal_ws));
}
