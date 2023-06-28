#version 460

in vec2 v_uv;

uniform sampler2D u_diffuse_texture;
uniform sampler2D u_specular_texture;
uniform sampler2D u_albedo_texture;
uniform sampler2D u_emissive_texture;
uniform sampler2D u_depth_texture;
uniform samplerCube u_skybox_texture;
uniform sampler2D u_shadow_texture;

uniform mat4 u_inverse_vp_matrix;

out layout(location = 0) vec3 o_colour;

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

void main()
{
    const float depth = texture(u_depth_texture, v_uv).r;
    if (depth < 1.f)
    {
        const vec3 diffuse  = texture(u_diffuse_texture, v_uv).rgb;
        const vec3 specular = texture(u_specular_texture, v_uv).rgb;
        const vec3 albedo   = texture(u_albedo_texture, v_uv).rgb;
        const vec3 emissive = texture(u_emissive_texture, v_uv).rgb;
        const float shadow = texture(u_shadow_texture, v_uv).r;
        const vec3 ambient  = 0.2f * albedo;

        o_colour = emissive + ambient + (1.f - shadow) * diffuse + (1.f - shadow) * specular;
    }
    else
    {
        o_colour = sample_skybox_colour();
    }
}
