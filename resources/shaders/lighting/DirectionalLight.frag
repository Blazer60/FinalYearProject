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

float sample_shadow_map(vec2 uv, float depth, float bias)
{
    const float shadow_depth = texture(u_shadow_map_texture, uv).r;
    return depth - bias > shadow_depth ? 1.f : 0.f;
}

float calculate_shadow_map(vec3 position, vec3 normal)
{
    const vec4 position_light_space = u_light_vp_matrix * vec4(position, 1.f);
    vec3 projection_coords = position_light_space.xyz / position_light_space.w;
    projection_coords = 0.5f * projection_coords + 0.5f;
    const float current_depth = projection_coords.z;
    const float bias = mix(0.001f, 0.f, light_dot(normal, -u_light_direction));

    if (current_depth < 1.f)
    {
        vec2 texel_size = 1.f / textureSize(u_shadow_map_texture, 0);
        float sum = 0.f;
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2(-1.5f, -1.5f)), current_depth, bias);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2( 1.5f, -1.5f)), current_depth, bias);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2(-1.5f,  1.5f)), current_depth, bias);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2( 1.5f,  1.5f)), current_depth, bias);
        return 0.25f * sum * dot(u_light_intensity, u_light_intensity) / 3.f;
    }
    return 0.f;
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