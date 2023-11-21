#version 460

in vec2 v_uv;

uniform sampler2D u_irradiance_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_depth_texture;
uniform samplerCube u_skybox_texture;
uniform sampler2D u_reflection_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_position_texture;
uniform sampler2D u_albedo_texture;
uniform sampler2D u_metallic_texture;
uniform sampler2D u_normal_texture;

uniform mat4 u_inverse_vp_matrix;
uniform float u_luminance_multiplier;
uniform float u_max_reflection_lod;
uniform vec3 u_camera_position_ws;

out layout(location = 0) vec3 o_colour;

const float PI = 3.14159265359f;

vec3 sample_skybox_colour()
{
    const vec2 uv_normalised = 2.f * v_uv - vec2(1.f);

    vec4 near_plane = vec4(uv_normalised.x, uv_normalised.y, -1.0f, 1.f);
    vec4 far_plane  = vec4(uv_normalised.x, uv_normalised.y,  1.0f, 1.f);

    near_plane = u_inverse_vp_matrix * near_plane;
    far_plane  = u_inverse_vp_matrix * far_plane;

    near_plane /= near_plane.w;
    far_plane  /= far_plane.w;

    const vec3 direction = (far_plane - near_plane).xyz;
    const vec3 colour = texture(u_skybox_texture, direction.xyz).rgb;
    return colour * u_luminance_multiplier;
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

vec3 localBrdf(vec2 hitUv)
{
    // Calculate all of the random variables we need.
    const vec3 albedo = texture(u_albedo_texture, v_uv).rgb;
    const vec3 position = texture(u_position_texture, v_uv).rgb;
    const float roughness = texture(u_roughness_texture, v_uv).r;
    const float metallic = texture(u_metallic_texture, v_uv).r;

    const vec3 hitPosition = texture(u_position_texture, hitUv).rgb;

    const vec3 l = normalize(hitPosition - position);  // Pretend that our reflection point is a light source direction.
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

    return diffuse + specular;
}

vec3 sampleReflections()
{
    const vec2 bufferSize = textureSize(u_reflection_texture, 0);
    const vec2 texelSize = 1.f / bufferSize;
    vec3 result = vec3(0.f);
    vec3 weightSum = vec3(0.f);

    vec2[] pixelOffsets = {
        vec2(-0.5f, -0.5f),
        vec2(-0.5f,  0.5f),
        vec2( 0.5f, -0.5f),
        vec2( 0.5f,  0.5f)
    };

    for (int i = 0; i < 4; ++i)
    {
        const vec2 pixelOffset = pixelOffsets[i];

        const vec4 data = texture(u_reflection_texture, v_uv + (pixelOffset * texelSize));
        if (data.w <= 0.f)  // The reflection ray didn't hit anything.
            return vec3(0.f);

        const vec2 hitUv = data.xy;
        const float pdf = data.z;

        const vec3 weight = localBrdf(hitUv) / pdf;
        result += texture(u_irradiance_texture, hitUv).rgb * weight;
        weightSum += weight;
    }

    result /= weightSum;
    return result;
}

void main()
{
    const float depth = texture(u_depth_texture, v_uv).r;
    if (depth < 1.f)
    {
        const vec3 l0  = texture(u_irradiance_texture, v_uv).rgb;
        const vec3 emissive = texture(u_emissive_texture, v_uv).rgb;
        const float roughness = texture(u_roughness_texture, v_uv).r;
        const float alpha = 1.f - roughness;
//        const vec3 reflection = alpha * alpha * textureLod(u_reflection_texture, v_uv, roughness * (u_max_reflection_lod - 1.f)).rgb;
        const vec3 reflection = sampleReflections().rgb;
        o_colour = emissive + l0 + reflection;
    }
    else
    {
        o_colour = sample_skybox_colour();
    }
}
