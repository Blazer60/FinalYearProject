#version 460

in vec2 v_uv;

uniform sampler2D u_albedo_texture;
uniform sampler2D u_position_texture;
uniform sampler2D u_normal_texture;
uniform sampler2DArray u_shadow_map_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metallic_texture;

uniform vec3 u_light_direction;
uniform vec3 u_light_intensity;
uniform mat4 u_light_vp_matrix[16];

uniform vec3 u_camera_position_ws;
uniform mat4 u_view_matrix;

uniform float u_cascade_distances[16];
uniform int u_cascade_count;

uniform vec2 u_bias;

out layout(location = 0) vec3 o_irradiance;

const float PI = 3.14159265359f;

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
        return 0.25f * sum * dot(u_light_intensity, u_light_intensity) / 3.f;
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

void main()
{
    // Calculate all of the random variables we need.
    const vec3 albedo = texture(u_albedo_texture, v_uv).rgb;
    const vec3 position = texture(u_position_texture, v_uv).rgb;
    const float roughness = texture(u_roughness_texture, v_uv).r;
    const float metallic = texture(u_metallic_texture, v_uv).r;

    const vec3 l = normalize(u_light_direction);
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

    // Calculate the intensity of the light.
    const float attenuation = 1.f;
    const float shadow_intensity = calculate_shadow_map(position, n);
    const vec3 radiance = (1.f - shadow_intensity) * u_light_intensity * attenuation;

    // Combine the output with dot(N, L).
    o_irradiance = irradiance * radiance * lDotN;
}
