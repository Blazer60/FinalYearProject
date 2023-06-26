#version 460

in vec2 v_uv;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_position_texture;
uniform sampler2D u_normal_texture;

uniform vec3 u_light_direction;
uniform vec3 u_light_intensity;

uniform vec3 u_camera_position_ws;

layout(location=0) out vec3 o_diffuse;
layout(location=1) out vec3 o_specular;

vec3 get_camera_direction()
{
    const vec3 position = texture(u_position_texture, v_uv).rgb;
    return normalize(u_camera_position_ws - position);
}

vec3 half_angle()
{
    return normalize(get_camera_direction() + u_light_direction);
}

float light_dot(vec3 a, vec3 b)
{
    return max(dot(a, b), 0.f);
}

void main()
{
    const vec3 albedo = texture(u_albedo_texture, v_uv).rgb;
    const vec3 normal = texture(u_normal_texture, v_uv).rgb;

    o_diffuse  = albedo * light_dot(normal, u_light_direction) * u_light_intensity;
    o_specular = albedo * pow(light_dot(half_angle(), normal), 128.f) * u_light_intensity;
}