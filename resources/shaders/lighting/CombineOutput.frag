#version 460

in vec2 v_uv;

uniform sampler2D u_irradiance_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_depth_texture;
uniform samplerCube u_skybox_texture;
uniform sampler2D u_reflection_texture;

uniform mat4 u_inverse_vp_matrix;
uniform float u_luminance_multiplier;
uniform float u_exposure;

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

void main()
{
    const float depth = texture(u_depth_texture, v_uv).r;
    if (depth < 1.f)
    {
        const vec3 l0  = texture(u_irradiance_texture, v_uv).rgb;
        const vec3 emissive = texture(u_emissive_texture, v_uv).rgb * u_exposure;
        const vec4 reflections = texture(u_reflection_texture, v_uv).rgba;
        o_colour = emissive + l0 + reflections.rgb;
    }
    else
    {
        o_colour = sample_skybox_colour() * u_exposure;
    }
}
