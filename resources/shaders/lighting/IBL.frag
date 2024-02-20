#version 460

#include "../geometry/GBuffer.glsl"
#include "../Camera.glsl"
#include "Brdf.glsl"
#include "../Maths.glsl"

#ifdef WHITE_FURNACE_TEST
    #include "../Colour.glsl"
#endif

in vec2 v_uv;

layout(binding = 0) uniform sampler2D depthBufferTexture;
layout(binding = 1) uniform samplerCube u_irradiance_texture;
layout(binding = 2) uniform samplerCube u_pre_filter_texture;
layout(binding = 3) uniform sampler2D u_brdf_lut_texture;

uniform float u_luminance_multiplier;

out layout(location = 0) vec3 o_irradiance;

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

#ifdef WHITE_FURNACE_TEST
    const vec3 fresnel = vec3(1.f);
#else
    const vec3 fresnel = fresnelSchlick(gBuffer.specular, vDotN);
#endif

    const vec2 lut = texture(u_brdf_lut_texture, vec2(vDotN, gBuffer.roughness)).rg;
    const vec3 specular = (fresnel * lut.x + lut.y);
    const vec3 diffuse = evaluateDiffuseBrdf(gBuffer, fresnel);

    const float maxReflectionLod = 4.f;
    const vec3 preFilterColour = textureLod(u_pre_filter_texture, r, gBuffer.roughness * maxReflectionLod).rgb;
    const vec3 irradiance = texture(u_irradiance_texture, n).rgb;

#ifdef WHITE_FURNACE_TEST
    o_irradiance = linearToSRgb((specular + diffuse) - vec3(0.5));
#else
    o_irradiance = camera.exposure * (specular * preFilterColour + diffuse * irradiance) * u_luminance_multiplier;
#endif
}
