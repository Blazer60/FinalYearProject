#version 460

#include "../interfaces/PointLightBlock.h"
#include "../interfaces/CameraBlock.h"
#include "Brdf.glsl"
#include "../geometry/GBuffer.glsl"

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_position_texture;
layout(binding = 1) uniform samplerCube u_shadow_map_texture;

out layout(location = 0) vec3 o_colour;

float sample_shadow_map(vec3 direction, float pixel_depth, float bias)
{
    const float shadow_depth = texture(u_shadow_map_texture, -direction).x * cLight.zFar;
    return pixel_depth - bias > shadow_depth ? 1.f : 0.f;
}

float calculate_shadow_map(vec3 light_direction)
{
    const vec3 samples_offsets[20] = vec3[20](
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );
    const float disk_radius = cLight.softnessRadius;

    float shadow = 0.f;
    for (int i = 0; i < 20; i++)
    {
        const vec3 direction = light_direction + samples_offsets[i] * disk_radius;
        const float pixel_depth = length(direction);
        const float bias = mix(cLight.bias.x, cLight.bias.y, clamp(pixel_depth / cLight.zFar, 0.f, 1.f));
        shadow += sample_shadow_map(direction, pixel_depth, bias);
    }

    return shadow / 20.f;
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

void main()
{
    const ivec2 coord = ivec2(floor(imageSize(storageGBuffer).xy * v_uv) + vec2(0.5f));
    GBuffer gBuffer = pullFromStorageGBuffer(coord);
    const vec3 position = texture(u_position_texture, v_uv).rgb;

    const vec3 light_direction = cLight.position - position;
    const vec3 l = normalize(light_direction);
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

    // Point light lighting calculation.
    float attenuation = 1.f;
    attenuation *= getDistanceAttenuation(light_direction, cLight.invSqrRadius);
    const float shadow_intensity = calculate_shadow_map(light_direction);
    const vec3 radiance = attenuation * cLight.intensity * (1.f - shadow_intensity);

    o_colour = camera.exposure * irradiance * radiance * (4.f * PI) * brdf.lDotN;
}
