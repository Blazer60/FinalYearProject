#version 460

in vec2 v_uv;

uniform sampler2D u_input_texture;

out layout(location = 0) vec3 o_colour;

void main()
{
    o_colour = vec3(1.f) - clamp(texture(u_input_texture, v_uv).rgb, vec3(0.f), vec3(1.f));
}