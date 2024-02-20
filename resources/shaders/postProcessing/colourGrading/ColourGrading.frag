#version 460 core

#include "../../Colour.glsl"

in vec2 v_uv;

uniform sampler2D u_texture;
uniform float u_inv_gamma_correction;
uniform float u_exposure;

out layout(location = 0) vec3 o_colour;

/**
 * https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
 */
vec3 narkowicz_aces(vec3 colour)
{
     return clamp((colour * (2.51f * colour + 0.03f)) / (colour * (2.43f * colour + 0.59f) + 0.14f), vec3(0.f), vec3(1.f));
}

void main()
{
    const vec3 texture_colour = texture(u_texture, v_uv).rgb;

    // https://www.youtube.com/watch?v=wbn5ULLtkHs
    // Exposure is just our colour value * exposure settings.
    const vec3 tone_map_colour = narkowicz_aces(texture_colour);  // The texture is already multiplied by exposure.
    const vec3 gamma_colour = linearToSRgb(tone_map_colour);

    o_colour = gamma_colour;
}
