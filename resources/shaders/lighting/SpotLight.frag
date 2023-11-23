#version 460

in vec2 v_uv;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_position_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metallic_texture;
uniform sampler2D u_shadow_map_texture;

uniform vec3 u_camera_position_ws;

uniform vec3 u_light_position;
uniform vec3 u_light_direction;
uniform float u_light_inv_sqr_radius;
uniform vec3 u_light_intensity;
uniform vec2 u_bias;
uniform float u_z_far;
//uniform float u_light_cut_off_angle;

uniform mat4 u_light_vp_matrix;

uniform float u_light_angle_scale;
uniform float u_light_angle_offset;

uniform float u_exposure;

out layout(location = 0) vec3 o_colour;

const float PI = 3.14159265359f;

float sample_shadow_map(vec2 uv, float depth, float bias)
{
    const float shadow_depth = texture(u_shadow_map_texture, uv).r;
    return depth - bias > shadow_depth ? 1.f : 0.f;
}

float calculate_shadow_map(vec3 light_direction, vec3 position, vec3 normal)
{
    const vec4 position_light_space = u_light_vp_matrix * vec4(position, 1.f);
    vec3 projection_coords = position_light_space.xyz / position_light_space.w;
    projection_coords = 0.5f * projection_coords + 0.5f;
    const float current_depth = length(light_direction) / u_z_far;
    const float bias = mix(u_bias.x, u_bias.y, max(dot(normal, -u_light_direction), 0.f));

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

float getAngleAttenuation(vec3 light_vector, float light_angle_scale, float light_angle_offset)
{
    float cd = dot(-u_light_direction, light_vector);
    float attenuation = clamp(cd * light_angle_scale + light_angle_offset, 0.f, 1.f);
    attenuation *= attenuation;

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
    attenuation *= getAngleAttenuation(l, u_light_angle_scale, u_light_angle_offset);
    attenuation *= getDistanceAttenuation(light_direction, u_light_inv_sqr_radius);
    float shadow_intensity = calculate_shadow_map(light_direction, position, n);
    const vec3 radiance = (1.f - shadow_intensity) * attenuation * u_light_intensity;

    o_colour = u_exposure * irradiance * radiance * (4.f * PI) * lDotN;
}
