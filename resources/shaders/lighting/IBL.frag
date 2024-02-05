#version 460

in vec2 v_uv;

layout(binding = 0) uniform sampler2D u_albedo_texture;
layout(binding = 1) uniform sampler2D u_position_texture;
layout(binding = 2) uniform sampler2D u_normal_texture;
layout(binding = 3) uniform sampler2D u_roughness_texture;
layout(binding = 4) uniform sampler2D u_metallic_texture;
layout(binding = 5) uniform samplerCube u_irradiance_texture;
layout(binding = 6) uniform samplerCube u_pre_filter_texture;
layout(binding = 7) uniform sampler2D u_brdf_lut_texture;

uniform vec3 u_camera_position_ws;

uniform float u_luminance_multiplier;

out layout(location = 0) vec3 o_irradiance;

// Unreal's fresnel function using spherical gaussian approximation.
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.f - roughness), f0) - f0) * pow(2.f, (-5.55473f * cosTheta - 6.98316f) * cosTheta);
}

void main()
{
    const vec3 albedo   = texture(u_albedo_texture, v_uv).rgb;
    const vec3 position = texture(u_position_texture, v_uv).rgb;
    const float metallic = texture(u_metallic_texture, v_uv).r;
    const float roughness = texture(u_roughness_texture, v_uv).r;

    const vec3 n = normalize(texture(u_normal_texture, v_uv).rgb);
    const vec3 v = normalize(u_camera_position_ws - position);
    const vec3 r = reflect(-v, n);

    const float vDotN = max(dot(v, n), 0.f);

    const vec3 f0 = mix(vec3(0.04f), albedo, metallic);

    const vec3 fresnel = fresnelSchlickRoughness(vDotN, f0, roughness);

    const vec3 kD = (vec3(1.f) - fresnel) * (1.f - metallic);

    const vec3 irradiance = texture(u_irradiance_texture, n).rgb;
    const vec3 diffuse = kD * irradiance * albedo;

    const float maxReflectionLod = 4.f;
    const vec3 preFilterColour = textureLod(u_pre_filter_texture, r, roughness * maxReflectionLod).rgb;
    const vec2 brdf = texture(u_brdf_lut_texture, vec2(vDotN, roughness)).rg;
    const vec3 specular = preFilterColour * (fresnel * brdf.x + brdf.y);

    o_irradiance = (specular + diffuse) * u_luminance_multiplier;
}
