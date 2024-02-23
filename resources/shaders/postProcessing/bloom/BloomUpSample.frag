#version 460

in vec2 v_uv;

uniform sampler2D u_up_sample_texture;
uniform sampler2D u_down_sample_texture;

uniform int u_down_mip_level;
uniform int u_up_mip_level;
uniform float u_scale;

out layout(location = 0) vec4 o_output;

vec3 up_filter_9_tap(sampler2D previous, vec2 uv, vec2 texel_size)
{
    vec3 texel_a = textureLod(previous, uv + texel_size * vec2(-1.f, -1.f) * u_scale, u_up_mip_level).rgb;
    vec3 texel_b = textureLod(previous, uv + texel_size * vec2(+0.f, -1.f) * u_scale, u_up_mip_level).rgb * 2.f;
    vec3 texel_c = textureLod(previous, uv + texel_size * vec2(+1.f, -1.f) * u_scale, u_up_mip_level).rgb;
    vec3 texel_d = textureLod(previous, uv + texel_size * vec2(-1.f, +0.f) * u_scale, u_up_mip_level).rgb * 2.f;
    vec3 texel_e = textureLod(previous, uv,                                           u_up_mip_level).rgb * 4.f;
    vec3 texel_f = textureLod(previous, uv + texel_size * vec2(+1.f, +0.f) * u_scale, u_up_mip_level).rgb * 2.f;
    vec3 texel_g = textureLod(previous, uv + texel_size * vec2(-1.f, +1.f) * u_scale, u_up_mip_level).rgb;
    vec3 texel_h = textureLod(previous, uv + texel_size * vec2(+0.f, +1.f) * u_scale, u_up_mip_level).rgb * 2.f;
    vec3 texel_i = textureLod(previous, uv + texel_size * vec2(+1.f, +1.f) * u_scale, u_up_mip_level).rgb;

    return (texel_a + texel_b + texel_c + texel_d + texel_e + texel_f + texel_g + texel_h + texel_i) / 16.f;
}

void main()
{
    vec2 texel_size = 1.f / textureSize(u_up_sample_texture, u_up_mip_level);
    o_output = vec4(max(up_filter_9_tap(u_up_sample_texture, v_uv, texel_size), vec3(0.f))
             + textureLod(u_down_sample_texture, v_uv, u_down_mip_level).rgb, 1.f);
}
