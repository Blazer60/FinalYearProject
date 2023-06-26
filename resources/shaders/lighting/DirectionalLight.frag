#version 460

in vec2 v_uv;

uniform sampler2D u_texture;

layout(location=0) out vec3 o_diffuse;

void main()
{
    vec3 colour = texture(u_texture, v_uv).rgb;
    o_diffuse = colour;
}