#version 460

in vec2 v_uv;

uniform sampler2D u_texture;
uniform int u_mip_level;

out layout(location = 0) vec3 o_output;

// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
vec3 down_filter_13_tap(vec2 uv, vec2 texel_size)
{
    // Unity uses a range of [-1, 1] using 0.5.
    // https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.postprocessing/PostProcessing/Shaders/Sampling.hlsl
    // https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.postprocessing/PostProcessing/Shaders/Builtins/Bloom.shader
    vec3 texel_a = textureLod(u_texture, uv + texel_size * vec2(-2, +2), u_mip_level).rgb;
    vec3 texel_b = textureLod(u_texture, uv + texel_size * vec2(+0, +2), u_mip_level).rgb;
    vec3 texel_c = textureLod(u_texture, uv + texel_size * vec2(+2, +2), u_mip_level).rgb;
    vec3 texel_d = textureLod(u_texture, uv + texel_size * vec2(-1, +1), u_mip_level).rgb;
    vec3 texel_e = textureLod(u_texture, uv + texel_size * vec2(+1, +1), u_mip_level).rgb;
    vec3 texel_f = textureLod(u_texture, uv + texel_size * vec2(-2, +0), u_mip_level).rgb;
    vec3 texel_g = textureLod(u_texture, uv,                             u_mip_level).rgb;
    vec3 texel_h = textureLod(u_texture, uv + texel_size * vec2(+2, +0), u_mip_level).rgb;
    vec3 texel_i = textureLod(u_texture, uv + texel_size * vec2(-1, -1), u_mip_level).rgb;
    vec3 texel_j = textureLod(u_texture, uv + texel_size * vec2(+1, -1), u_mip_level).rgb;
    vec3 texel_k = textureLod(u_texture, uv + texel_size * vec2(-2, -2), u_mip_level).rgb;
    vec3 texel_l = textureLod(u_texture, uv + texel_size * vec2(+0, -2), u_mip_level).rgb;
    vec3 texel_m = textureLod(u_texture, uv + texel_size * vec2(+2, -2), u_mip_level).rgb;

    vec3 sector_a = 0.125f * (texel_a + texel_b + texel_f + texel_g) / 4.f;
    vec3 sector_b = 0.125f * (texel_b + texel_c + texel_g + texel_h) / 4.f;
    vec3 sector_c = 0.5f   * (texel_d + texel_e + texel_i + texel_j) / 4.f;
    vec3 sector_d = 0.125f * (texel_f + texel_g + texel_k + texel_l) / 4.f;
    vec3 sector_e = 0.125f * (texel_g + texel_h + texel_l + texel_m) / 4.f;

    return sector_a + sector_b + sector_c + sector_d + sector_e;
}

void main()
{
    const vec2 texel_size = 1.f / textureSize(u_texture, u_mip_level);
    o_output = max(down_filter_13_tap(v_uv, texel_size), vec3(0.f));
}
