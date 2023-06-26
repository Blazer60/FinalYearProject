#version 460

in vec2 v_uv;

uniform sampler2D u_diffuse_texture;
uniform sampler2D u_specular_texture;
uniform sampler2D u_albedo_texture;
uniform sampler2D u_emissive_texture;

out layout(location = 0) vec3 o_colour;

void main()
{
    const vec3 diffuse  = texture(u_diffuse_texture, v_uv).rgb;
    const vec3 specular = texture(u_specular_texture, v_uv).rgb;
    const vec3 albedo   = texture(u_albedo_texture, v_uv).rgb;
    const vec3 emissive = texture(u_emissive_texture, v_uv).rgb;
    const vec3 ambient  = 0.2f * albedo;

    o_colour = emissive + ambient + diffuse + specular;
}
