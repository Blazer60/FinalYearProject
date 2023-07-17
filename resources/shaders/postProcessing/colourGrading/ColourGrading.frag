#version 460 core

in vec2 v_uv;

uniform sampler2D u_texture;
uniform float u_inv_gamma_correction;
uniform float u_exposure;

uniform float u_slope;
uniform float u_toe;
uniform float u_shoulder;
uniform float u_black_clip;
uniform float u_white_clip;

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

vec3 aces(vec3 colour)
{
     return (colour * (2.51f * colour + 0.03f)) / (colour * (2.43f * colour + 0.59f) + 0.14f);
}

vec3 aces_tone_map(vec3 colour)
{
    const vec3 c = clamp(pow(vec3(10.f), colour), 0.f, u_exposure * 100.f);
    return u_slope * aces(0.8f * c);
}

void main()
{
    const vec3 texture_colour = texture(u_texture, v_uv).rgb;
    const vec3 tone_map_colour = aces_tone_map(texture_colour - vec3(u_exposure));
    const vec3 gamma_colour = linearToSRgb(tone_map_colour);

    o_colour = gamma_colour;
}
