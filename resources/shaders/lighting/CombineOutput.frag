#version 460

in vec2 v_uv;

uniform sampler2D u_diffuse_texture;
uniform sampler2D u_position_texture;
uniform sampler2D u_albedo_texture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_depth_texture;
uniform samplerCube u_skybox_texture;
uniform samplerCube u_irradiance_texture;
uniform sampler2D u_shadow_texture;
uniform sampler2D u_metallic_texture;
uniform sampler2D u_roughness_texture;
uniform samplerCube u_pre_filter_texture;
uniform sampler2D u_brdf_lut_texture;

uniform mat4 u_inverse_vp_matrix;
uniform vec3 u_camera_position_ws;

out layout(location = 0) vec3 o_colour;

vec3 get_camera_direction(vec3 position)
{
    return normalize(u_camera_position_ws - position);
}

vec3 linearToSRgb(vec3 linear)
{
    const vec3 inverseGamma = vec3(1.f / 2.2f);
    return pow(linear, inverseGamma);
}

vec3 sRgbToLinear(vec3 sRgb)
{
    const vec3 gamma = vec3(2.2f);
    return pow(sRgb, gamma);
}

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
    return colour;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    // FSchlick(h,v,F0)=F0+(1−F0)(1−(h⋅v))^5
    return f0 + (max(vec3(1.f - roughness), f0) - f0) * pow(clamp(1.f - cosTheta, 0.f, 1.f), 5.f);
}

vec3 toneMap(vec3 colour)
{
    return colour;
//    return colour / (vec3(1.f) + colour);
}

void main()
{
    const float depth = texture(u_depth_texture, v_uv).r;
    if (depth < 1.f)
    {
        const vec3 l0  = texture(u_diffuse_texture, v_uv).rgb;
        const vec3 albedo   = texture(u_albedo_texture, v_uv).rgb;
        const vec3 position = texture(u_position_texture, v_uv).rgb;
        const vec3 normal = texture(u_normal_texture, v_uv).rgb;
        const vec3 emissive = texture(u_emissive_texture, v_uv).rgb;
        const float shadow = texture(u_shadow_texture, v_uv).r;
        const float metallic = texture(u_metallic_texture, v_uv).r;
        const float roughness = texture(u_roughness_texture, v_uv).r;

        vec3 f0 = vec3(0.04f);
        f0 = mix(f0, albedo, metallic);

        const vec3 V = get_camera_direction(position);
        const vec3 N = normalize(normal);
        const vec3 R = reflect(-V, N);

        const vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.f), f0, roughness);

        const vec3 kS = F;
        vec3 kD = vec3(1.f) - kS;
        kD *= 1.f - metallic;

        const vec3 irradiance = texture(u_irradiance_texture, N).rgb;
        const vec3 diffuse = irradiance * albedo;

        const float maxReflectionLod = 4.f;
        const vec3 preFilterColour = textureLod(u_pre_filter_texture, R, roughness * maxReflectionLod).rgb;
        const vec2 brdf = texture(u_brdf_lut_texture, vec2(max(dot(N, V), 0.f), roughness)).rg;
        const vec3 specular = preFilterColour * (F * brdf.x + brdf.y);

        const vec3 ambient = kD * diffuse + specular;

        o_colour = emissive + ambient + l0;
    }
    else
    {
        o_colour = sample_skybox_colour();
    }
}
