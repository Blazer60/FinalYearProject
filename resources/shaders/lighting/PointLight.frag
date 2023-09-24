#version 460

in vec2 v_uv;

uniform sampler2D u_normal_texture;

out layout(location = 0) vec3 o_colour;

void main()
{
    const vec3 n = normalize(texture(u_normal_texture, v_uv).rgb);
    o_colour = n;
}
