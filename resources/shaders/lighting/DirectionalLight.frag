#version 460

in vec2 v_uv;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_position_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_shadow_map_texture;

uniform vec3 u_light_direction;
uniform vec3 u_light_intensity;
uniform mat4 u_light_vp_matrix;

uniform vec3 u_camera_position_ws;

layout(location=0) out vec3 o_diffuse;
layout(location=1) out vec3 o_specular;
layout(location=2) out float o_shadow;

vec3 get_camera_direction(vec3 position)
{
    return normalize(u_camera_position_ws - position);
}

vec3 half_angle(vec3 position)
{
    return normalize(get_camera_direction(position) + u_light_direction);
}

float light_dot(vec3 a, vec3 b)
{
    return max(dot(a, b), 0.f);
}

float calculate_shadow_map(vec3 position, vec3 normal)
{
    const vec4 position_light_space = u_light_vp_matrix * vec4(position, 1.f);
    vec3 projection_coords = position_light_space.xyz / position_light_space.w;
    projection_coords = 0.5f * projection_coords + 0.5f;

    const float shadow_depth = texture(u_shadow_map_texture, projection_coords.xy).r;
    const float current_depth = projection_coords.z;
    const float bias = mix(0.005f, 0.f, light_dot(normal, -u_light_direction));
    float shadow = 0.f;
    if (projection_coords.z < 1.f)
        shadow = current_depth - bias > shadow_depth ? 1.f : 0.f;
    return shadow;
}

void main()
{
    const vec3 albedo = texture(u_albedo_texture, v_uv).rgb;
    const vec3 normal = texture(u_normal_texture, v_uv).rgb;
    const vec3 position = texture(u_position_texture, v_uv).rgb;

    o_diffuse  = albedo * light_dot(normal, u_light_direction) * u_light_intensity;
    // 128.f should be placed into a specular texture that is read in.
    o_specular = albedo * pow(light_dot(half_angle(position), normal), 128.f) * u_light_intensity;
    o_shadow = calculate_shadow_map(position, normal);
}