#version 460

#include "../interfaces/CameraBlock.h"
#include "../interfaces/SpotlightBlock.h"
#include "Brdf.glsl"
#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"

in vec2 v_uv;

layout(binding = 0) uniform sampler2D depthBufferTexture;
layout(binding = 1) uniform sampler2D u_shadow_map_texture;

out layout(location = 0) vec3 o_colour;

float sample_shadow_map(vec2 uv, float depth, float bias)
{
    const float shadow_depth = texture(u_shadow_map_texture, uv).r;
    return depth - bias > shadow_depth ? 1.f : 0.f;
}

float calculate_shadow_map(vec3 light_direction, vec3 position, vec3 normal)
{
    const vec4 position_light_space = cLight.vpMatrix * vec4(position, 1.f);
    vec3 projection_coords = position_light_space.xyz / position_light_space.w;
    projection_coords = 0.5f * projection_coords + 0.5f;
    const float current_depth = length(light_direction) / cLight.zFar;
    const float bias = mix(cLight.bias.x, cLight.bias.y, max(dot(normal, -cLight.direction), 0.f));

    if (current_depth < 1.f)
    {
        vec2 texel_size = 1.f / textureSize(u_shadow_map_texture, 0).xy;
        float sum = 0.f;
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2(-0.5f, -0.5f)), current_depth, bias);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2( 0.5f, -0.5f)), current_depth, bias);
        sum += sample_shadow_map(projection_coords.xy,                                     current_depth, bias);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2(-0.5f,  0.5f)), current_depth, bias);
        sum += sample_shadow_map(projection_coords.xy + (texel_size * vec2( 0.5f,  0.5f)), current_depth, bias);
        return 0.2f * sum;
    }
    return 0.f;
}

float smoothDistanceAttenuation(float distance2, float invSqrAttRadius)
{
    const float factor = distance2 * invSqrAttRadius;
    const float smoothFactor = clamp(1.f - factor * factor, 0.f, 1.f);
    return smoothFactor * smoothFactor;
}

float getDistanceAttenuation(vec3 lightVector, float invSqrAttRadius)
{
    const float distance2 = dot(lightVector, lightVector);
    float attenuation = 1.f / (max(distance2, 0.01f * 0.01f));  // Point lights are considered to have a radius of 1cm.
    attenuation *= smoothDistanceAttenuation(distance2, invSqrAttRadius);

    return attenuation;
}

float getAngleAttenuation(vec3 light_vector, float light_angle_scale, float light_angle_offset)
{
    float cd = dot(-cLight.direction, light_vector);
    float attenuation = clamp(cd * light_angle_scale + light_angle_offset, 0.f, 1.f);
    attenuation *= attenuation;

    return attenuation;
}

void main()
{
    const float depth = texture(depthBufferTexture, v_uv).r;
    const vec3 position = positionFromDepth(v_uv, depth);

    const ivec2 coord = ivec2(floor(imageSize(storageGBuffer).xy * v_uv) + vec2(0.5f));
    GBuffer gBuffer = pullFromStorageGBuffer(coord);

    const vec3 light_direction = cLight.position - position;
    const vec3 l = normalize(light_direction);
    const vec3 n = gBuffer.normal;
    const vec3 v = normalize(camera.position - position);
    const vec3 h = normalize(l + v);

    const float vDotN = max(dot(v, n), 0.f);
    const float lDotN = max(dot(l, n), 0.f);
    const float vDotH = max(dot(v, h), 0.f);
    const float nDotH = max(dot(n, h), 0.f);

    Brdf brdf;
    brdf.vDotN = max(dot(v, n), 0.f);
    brdf.lDotN = max(dot(l, n), 0.f);
    brdf.vDotH = max(dot(v, h), 0.f);
    brdf.nDotH = max(dot(n, h), 0.f);
    brdf.albedo = gBuffer.diffuse;
    brdf.f0 = gBuffer.specular;
    brdf.roughness = gBuffer.roughness;

    const vec3 irradiance = calculateIrradiance(brdf);

    // Point light lighting calculation.
    float attenuation = 1.f;
    attenuation *= getAngleAttenuation(l, cLight.angleScale, cLight.angleOffset);
    attenuation *= getDistanceAttenuation(light_direction, cLight.invSqrRadius);
    float shadow_intensity = calculate_shadow_map(light_direction, position, n);
    const vec3 radiance = (1.f - shadow_intensity) * attenuation * cLight.intensity;

    o_colour = camera.exposure * irradiance * radiance * (4.f * PI) * brdf.lDotN;
}
