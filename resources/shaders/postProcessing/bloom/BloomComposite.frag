#version 460

in vec2 v_uv;

uniform sampler2D u_original;
uniform sampler2D u_bloom;
uniform vec3 u_strength;

out layout(location = 0) vec3 o_output;

void main()
{
    const vec3 original_colour = max(texture(u_original, v_uv).rgb, vec3(0.f));
    const vec3 bloom_colour = max(textureLod(u_bloom, v_uv, 1.f).rgb, vec3(0.f));

    o_output = mix(original_colour, bloom_colour, u_strength);
}
