#version 460

in vec2 v_uv;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_position_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metallic_texture;

uniform vec3 u_light_position;
uniform vec3 u_light_intensity;
uniform float u_light_inv_sqr_radius;

uniform vec3 u_camera_position_ws;

out layout(location = 0) vec3 o_colour;

const float PI = 3.14159265359f;

// Unreal's fresnel function using spherical gaussian approximation.
vec3 fresnelSchlick(float vDotH, vec3 f0)
{
    return f0 + (1.f - f0) * pow(2.f, (-5.55473f * vDotH - 6.98316f) * vDotH);
}

// Unreal's and Disney's distribution function.
float distributionGgx(float nDotH, float roughness)
{
    const float alpha = roughness * roughness;
    const float alpha2 = alpha * alpha;
    const float nDotH2 = nDotH * nDotH;
    const float denominator = nDotH2 * (alpha2 - 1.f) + 1.f;

    return alpha2 / (PI * denominator * denominator);
}

// Unreal's geometry schlick function.
float geometrySchlick(float vDotN, float k)
{
    return vDotN / (vDotN * (1.f - k) + k);
}

// Unreal's geometry smith function. Note the remapping of roughness before squaring
// should only be done on analytical lights. Remap function: (R + 1) / 2
float geometrySmith(float vDotN, float lDotN, float roughness)
{
    const float r = roughness + 1.f;
    const float k = r * r / 8.f;
    return geometrySchlick(vDotN, k) * geometrySchlick(lDotN, k);
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
    const vec3 albedo = texture(u_albedo_texture, v_uv).rgb;
    const vec3 position = texture(u_position_texture, v_uv).rgb;
    const float roughness = texture(u_roughness_texture, v_uv).r;
    const float metallic = texture(u_metallic_texture, v_uv).r;

    const vec3 light_direction = u_light_position - position;
    const vec3 l = normalize(light_direction);
    const vec3 n = normalize(texture(u_normal_texture, v_uv).rgb);
    const vec3 v = normalize(u_camera_position_ws - position);
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

    // Point light lighting calculation.
    float attenuation = 1.f;
    attenuation *= getDistanceAttenuation(light_direction, u_light_inv_sqr_radius);

    o_colour = irradiance * attenuation * u_light_intensity / (4.f * PI) * lDotN;
}