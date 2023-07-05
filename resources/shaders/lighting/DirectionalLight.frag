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

layout(location=0) out vec3 o_diffuse;
layout(location=1) out float o_shadow;

const float PI = 3.14159265359f;

vec3 get_camera_direction(vec3 position)
{
    return normalize(u_camera_position_ws - position);
}

vec3 half_angle(vec3 position)
{
    return normalize(get_camera_direction(position) + u_light_direction);
}

float light_dot(vec3 a, vec3 b)
{
    return max(dot(a, b), 0.f);
}

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
    const float bias = mix(u_bias.x, u_bias.y, light_dot(normal, -u_light_direction));

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

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    // FSchlick(h,v,F0)=F0+(1−F0)(1−(h⋅v))^5
    return f0 + (1.f - f0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

float distributionGgx(vec3 N, vec3 H, float roughness)
{
    const float a = roughness * roughness;
    const float a2 = a * a;
    const float nDotH = max(dot(N, H), 0.f);
    const float nDotH2 = nDotH * nDotH;
    const float denominator = (nDotH2 * (a2 - 1.f) + 1.f);

    return a2 / (PI * denominator * denominator);
}

float geometrySchlickGgx(float nDotV, float roughness)
{
    const float r = roughness + 1.f;
    const float k = r * r / 8.f;

    return nDotV / (nDotV * (1.f - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    const float nDotV = max(dot(N, V), 0.f);
    const float nDotL = max(dot(N, L), 0.f);
    const float ggx2 = geometrySchlickGgx(nDotV, roughness);
    const float ggx1 = geometrySchlickGgx(nDotL, roughness);

    return ggx1 * ggx2;
}


void main()
{
    const vec3 albedo = texture(u_albedo_texture, v_uv).rgb;
    const vec3 normal = texture(u_normal_texture, v_uv).rgb;
    const vec3 position = texture(u_position_texture, v_uv).rgb;
    const float roughness = texture(u_roughness_texture, v_uv).r;
    const float metallic = texture(u_metallic_texture, v_uv).r;

    const vec3 wi = normalize(u_light_direction);
    const float cosTheta = max(dot(normal, wi), 0.f);
    const float attenuation = 1.f;
    const vec3 radiance = u_light_intensity * attenuation * cosTheta;

    const vec3 H = half_angle(position);
    const vec3 V = get_camera_direction(position);
    const vec3 N = normalize(normal);
    const vec3 L = wi;

    vec3 f0 = vec3(0.04f);
    f0 = mix(f0, albedo, metallic);
    const vec3 F = fresnelSchlick(max(dot(H, V), 0.f), f0);
    const float NDF = distributionGgx(N, H, roughness);
    const float G = geometrySmith(N, V, L, roughness);
    const vec3 specular = NDF * G * F / (4.f * max(dot(N, V), 0.f) * max(dot(N, L), 0.f) + 0.0001f);

    const vec3 kS = F;
    vec3 kD = vec3(1.f) - kS;
    kD *= 1.f - metallic;

    const float nDotL = max(dot(N, L), 0.f);
    const vec3 lo = (kD * albedo / PI + specular) * radiance * nDotL;

    o_diffuse  = lo;
    o_shadow = calculate_shadow_map(position, normal);
}