#version 460 core

in vec2 v_uv;

uniform sampler2D u_texture;
uniform float u_inv_gamma_correction;
uniform float u_exposure;

out layout(location = 0) vec3 o_colour;

vec3 linearToSRgb(vec3 linear)
{
    return pow(linear, vec3(u_inv_gamma_correction));
}

vec3 reinhard_tone_mapping(vec3 colour)
{
    return colour / (colour + vec3(1.f));
}

vec3 exposure_tone_mapping(vec3 colour)
{
    return vec3(1.f) - exp(-colour * u_exposure);
}

void main()
{
    const vec3 texture_colour = texture(u_texture, v_uv).rgb;
    const vec3 tone_map_colour = exposure_tone_mapping(texture_colour);
    const vec3 gamma_colour = linearToSRgb(tone_map_colour);

    o_colour = gamma_colour;
}
