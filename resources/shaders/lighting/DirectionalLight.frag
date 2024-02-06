#version 460 core

#include "../interfaces/CameraBlock.h"
#include "Brdf.glsl"

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_albedo_texture;
layout(binding = 1) uniform sampler2D u_position_texture;
layout(binding = 2) uniform sampler2D u_normal_texture;
layout(binding = 3) uniform sampler2DArray u_shadow_map_texture;
layout(binding = 4) uniform sampler2D u_roughness_texture;
layout(binding = 5) uniform sampler2D u_metallic_texture;

uniform vec3 u_light_direction;
uniform vec3 u_light_intensity;
uniform mat4 u_light_vp_matrix[16];

uniform vec3 u_camera_position_ws;
uniform mat4 u_view_matrix;

uniform float u_cascade_distances[16];
uniform int u_cascade_count;

uniform vec2 u_bias;

uniform float u_exposure;

out layout(location = 0) vec3 o_irradiance;

float sample_shadow_map(vec2 uv, float depth, float bias, int layer)
{
    const float shadow_depth = texture(u_shadow_map_texture, vec3(uv, layer)).r;
    return depth - bias > shadow_depth ? 1.f : 0.f;
}

float calculate_shadow_map(vec3 position, vec3 normal)
{
    const float depth = abs((u_view_matrix * vec4(position, 1.f)).z);
    int layer = u_cascade_count;
    for (int i = 0; i < u_cascade_count; ++i)
    {
        if (depth < u_cascade_distances[i])
        {
            layer = i;
            break;
        }
    }

    const vec4 position_light_space = u_light_vp_matrix[layer] * vec4(position, 1.f);
    vec3 projection_coords = position_light_space.xyz / position_light_space.w;
    projection_coords = 0.5f * projection_coords + 0.5f;
    const float current_depth = projection_coords.z;
    const float bias = mix(u_bias.x, u_bias.y, max(dot(normal, -u_light_direction), 0.f));

    if (current_depth < 1.f)
    {
        vec2 texel_size = 1.f / textureSize(u_shadow_map_texture, 0).xy;
        float sum = 0.f;
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2(-1.5f, -1.5f)), current_depth, bias, layer);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2( 1.5f, -1.5f)), current_depth, bias, layer);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2(-1.5f,  1.5f)), current_depth, bias, layer);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2( 1.5f,  1.5f)), current_depth, bias, layer);
        return 0.25f * sum;
    }
    return 0.f;
}


void main()
{
    // Calculate all of the random variables we need.
    const vec3 albedo = texture(u_albedo_texture, v_uv).rgb;
    const vec3 position = texture(u_position_texture, v_uv).rgb;
    const float roughness = texture(u_roughness_texture, v_uv).r;
    const float metallic = texture(u_metallic_texture, v_uv).r;

    const vec3 l = normalize(u_light_direction);
    const vec3 n = normalize(texture(u_normal_texture, v_uv).rgb);
    const vec3 v = normalize(camera.position - position);
    const vec3 h = normalize(l + v);

    const float vDotN = max(dot(v, n), 0.f);
    const float lDotN = max(dot(l, n), 0.f);
    const float vDotH = max(dot(v, h), 0.f);
    const float nDotH = max(dot(n, h), 0.f);

    const vec3 f0 = mix(vec3(0.04f), albedo, metallic);

    // Calculate how the material interacts with the light.
    const vec3 fresnel = fresnelSchlick(vDotH, f0);
    const float distribution = distributionGgx(nDotH, roughness);
    const float geometry = geometrySmith(vDotN, lDotN, roughness);

    const vec3 specular = distribution * geometry * fresnel / (4.f * vDotN * lDotN + 0.0001f);

    const vec3 kD = (vec3(1.f) - fresnel) * (1.f - metallic);
    const vec3 diffuse = kD * albedo / PI;

    const vec3 irradiance = diffuse + specular;

    // Calculate the intensity of the light.
    const float attenuation = 1.f;
    const float shadow_intensity = calculate_shadow_map(position, n);
    const vec3 radiance = (1.f - shadow_intensity) * u_light_intensity * attenuation;

    // Combine the output with dot(N, L).
    o_irradiance = u_exposure * irradiance * radiance * lDotN;
}
