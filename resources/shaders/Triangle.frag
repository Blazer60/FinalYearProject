#version 460 core

in vec2 v_uv;

layout(location = 0) out vec3 o_colour;

uniform vec3 u_colour;
uniform sampler2D u_texture;

void main()
{
    vec3 texture_colour = texture(u_texture, v_uv).xyz;
    if (texture_colour == vec3(0.f))
        texture_colour = vec3(1.f);

    o_colour.rgb = u_colour * texture_colour;
}