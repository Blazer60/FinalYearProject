#version 460

#include "../../interfaces/CameraBlock.h"

layout(location=0) in vec3 a_position;
layout(location=1) in vec2 a_uv;
layout(location=2) in vec3 a_normal;
layout(location=3) in vec3 a_tangent;

uniform mat4 u_mvp_matrix;
uniform mat4 u_model_matrix;

out vec2 v_uv;
out vec3 v_position_ws;
out vec3 v_normal_ws;
out flat mat3 v_tbn_matrix;
out vec3 v_camera_position_ts;
out vec3 v_position_ts;
out vec2 vScreenUv;

void main()
{
    gl_Position = u_mvp_matrix * vec4(a_position, 1.f);
    const vec3 ndc = gl_Position.xyz / gl_Position.w;
    vScreenUv = ndc.xy * 0.5f + 0.5f;

    v_uv = a_uv;
    v_normal_ws = normalize(vec3(u_model_matrix * vec4(a_normal, 0.f)));
    v_position_ws = vec3(u_model_matrix * vec4(a_position.xyz, 1.f));

    const vec3 tangent_ws = normalize(vec3(u_model_matrix * vec4(a_tangent, 0.f)));
    const vec3 normal_ws = normalize(vec3(u_model_matrix * vec4(a_normal, 0.f)));
    const vec3 bi_tangent_ws = cross(normal_ws, tangent_ws);

    v_tbn_matrix = (mat3(tangent_ws, bi_tangent_ws, normal_ws));

    const mat3 transposed_tbn_matrix = transpose(v_tbn_matrix);
    v_camera_position_ts = transposed_tbn_matrix * camera.position;
    v_position_ts = transposed_tbn_matrix * v_position_ws;
}
