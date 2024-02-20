#version 460 core

#include "../interfaces/CameraBlock.h"
#include "../interfaces/DirectionalLightBlock.h"
#include "Brdf.glsl"
#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"

in vec2 v_uv;

layout(binding = 0) uniform sampler2D depthBufferTexture;
layout(binding = 1) uniform sampler2DArray u_shadow_map_texture;

out layout(location = 0) vec3 o_irradiance;

float sample_shadow_map(vec2 uv, float depth, float bias, int layer)
{
    const float shadow_depth = texture(u_shadow_map_texture, vec3(uv, layer)).r;
    return depth - bias > shadow_depth ? 1.f : 0.f;
}

float calculate_shadow_map(vec3 position, vec3 normal)
{
    const float depth = abs((camera.viewMatrix * vec4(position, 1.f)).z);
    int layer = light.cascadeCount;
    for (int i = 0; i < light.cascadeCount; ++i)
    {
        if (depth < light.cascadeDistances[i / 4][i % 4])
        {
            layer = i;
            break;
        }
    }

    const vec4 position_light_space = light.vpMatrices[layer] * vec4(position, 1.f);
    vec3 projection_coords = position_light_space.xyz / position_light_space.w;
    projection_coords = 0.5f * projection_coords + 0.5f;
    const float current_depth = projection_coords.z;
    const float bias = mix(light.bias.x, light.bias.y, max(dot(normal, -light.direction), 0.f));

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
    const float depth = texture(depthBufferTexture, v_uv).r;
    const vec3 position = positionFromDepth(v_uv, depth);

    const ivec2 coord = ivec2(floor(imageSize(storageGBuffer).xy * v_uv) + vec2(0.5f));
    GBuffer gBuffer = pullFromStorageGBuffer(coord);

    const vec3 l = normalize(light.direction);
    const vec3 n = gBuffer.normal;
    const vec3 v = normalize(camera.position - position);
    const vec3 h = normalize(l + v);

    Brdf brdf;
    brdf.vDotN = max(dot(v, n), 0.f);
    brdf.lDotN = max(dot(l, n), 0.f);
    brdf.vDotH = max(dot(v, h), 0.f);
    brdf.nDotH = max(dot(n, h), 0.f);
    brdf.albedo = gBuffer.diffuse;
    brdf.f0 = gBuffer.specular;
    brdf.roughness = gBuffer.roughness;

    const vec3 irradiance = calculateIrradiance(brdf);

    const float attenuation = 1.f;
    const float shadow_intensity = calculate_shadow_map(position, n);
    const vec3 radiance = (1.f - shadow_intensity) * light.intensity * attenuation;

    // Combine the output with dot(N, L).
    o_irradiance = camera.exposure * irradiance * radiance * brdf.lDotN;
}
