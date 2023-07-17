#version 460

in vec2 v_uv;

uniform vec3 u_light_key_threshold;
uniform vec3 u_light_max_threshold;
uniform sampler2D u_texture;

out layout(location = 0) vec3 o_output;

void main()
{
    o_output = clamp(vec3(texture(u_texture, v_uv)) - u_light_key_threshold, vec3(0.f), u_light_max_threshold);
}
