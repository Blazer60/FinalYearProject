#version 460

#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"

in vec2 v_uv;

layout(binding = 0) uniform sampler2D depthBufferTexture;
layout(binding = 1) uniform samplerCube u_irradiance_texture;
layout(binding = 2) uniform samplerCube u_pre_filter_texture;
layout(binding = 3) uniform sampler2D u_brdf_lut_texture;

uniform float u_luminance_multiplier;

out layout(location = 0) vec3 o_irradiance;

// Unreal's fresnel function using spherical gaussian approximation.
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.f - roughness), f0) - f0) * pow(2.f, (-5.55473f * cosTheta - 6.98316f) * cosTheta);
}

void main()
{
    const float depth = texture(depthBufferTexture, v_uv).r;
    const vec3 position = positionFromDepth(v_uv, depth);

    const ivec2 coord = ivec2(floor(imageSize(storageGBuffer).xy * v_uv) + vec2(0.5f));
    GBuffer gBuffer = pullFromStorageGBuffer(coord);

    const vec3 n = gBuffer.normal;
    const vec3 v = normalize(camera.position - position);
    const vec3 r = reflect(-v, n);

    const float vDotN = max(dot(v, n), 0.f);

    const vec3 f0 = gBuffer.specular;

    const vec3 fresnel = fresnelSchlickRoughness(vDotN, f0, gBuffer.roughness);

    const vec3 kD = (vec3(1.f) - fresnel) * gBuffer.diffuse / PI;

    const vec3 irradiance = texture(u_irradiance_texture, n).rgb;
    const vec3 diffuse = kD * irradiance * gBuffer.diffuse;

    const float maxReflectionLod = 4.f;
    const vec3 preFilterColour = textureLod(u_pre_filter_texture, r, gBuffer.roughness * maxReflectionLod).rgb;
    const vec2 brdf = texture(u_brdf_lut_texture, vec2(vDotN, gBuffer.roughness)).rg;
    const vec3 specular = preFilterColour * (fresnel * brdf.x + brdf.y);

    o_irradiance = camera.exposure * (specular + diffuse) * u_luminance_multiplier;
}
