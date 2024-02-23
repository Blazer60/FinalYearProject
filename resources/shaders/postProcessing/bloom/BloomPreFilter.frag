#version 460

in vec2 v_uv;

uniform sampler2D u_texture;
uniform float u_exposure;

out layout(location = 0) vec4 o_output;

vec3 to_srgb(vec3 v)
{
    return pow(v, vec3(1.f / 2.2f));
}

float srgb_to_luminance(vec3 colour)
{
    return dot(colour, vec3(0.2126f, 0.7152f, 0.0722f));
}

float karis_average(vec3 colour)
{
    float luma = srgb_to_luminance(to_srgb(colour)) * 0.25f;
    return 1.f / (1.f + luma);
}

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
    const float lod = 0.f;
    // Unity uses a range of [-1, 1] using 0.5.
    // https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.postprocessing/PostProcessing/Shaders/Sampling.hlsl
    // https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.postprocessing/PostProcessing/Shaders/Builtins/Bloom.shader
    const vec3 texel_a = textureLod(u_texture, uv + texel_size * vec2(-2, +2), lod).rgb / u_exposure;
    const vec3 texel_b = textureLod(u_texture, uv + texel_size * vec2(+0, +2), lod).rgb / u_exposure;
    const vec3 texel_c = textureLod(u_texture, uv + texel_size * vec2(+2, +2), lod).rgb / u_exposure;
    const vec3 texel_d = textureLod(u_texture, uv + texel_size * vec2(-1, +1), lod).rgb / u_exposure;
    const vec3 texel_e = textureLod(u_texture, uv + texel_size * vec2(+1, +1), lod).rgb / u_exposure;
    const vec3 texel_f = textureLod(u_texture, uv + texel_size * vec2(-2, +0), lod).rgb / u_exposure;
    const vec3 texel_g = textureLod(u_texture, uv,                             lod).rgb / u_exposure;
    const vec3 texel_h = textureLod(u_texture, uv + texel_size * vec2(+2, +0), lod).rgb / u_exposure;
    const vec3 texel_i = textureLod(u_texture, uv + texel_size * vec2(-1, -1), lod).rgb / u_exposure;
    const vec3 texel_j = textureLod(u_texture, uv + texel_size * vec2(+1, -1), lod).rgb / u_exposure;
    const vec3 texel_k = textureLod(u_texture, uv + texel_size * vec2(-2, -2), lod).rgb / u_exposure;
    const vec3 texel_l = textureLod(u_texture, uv + texel_size * vec2(+0, -2), lod).rgb / u_exposure;
    const vec3 texel_m = textureLod(u_texture, uv + texel_size * vec2(+2, -2), lod).rgb / u_exposure;

    vec3 sector_a = 0.125f * (texel_a + texel_b + texel_f + texel_g) / 4.f;
    vec3 sector_b = 0.125f * (texel_b + texel_c + texel_g + texel_h) / 4.f;
    vec3 sector_c = 0.5f   * (texel_d + texel_e + texel_i + texel_j) / 4.f;
    vec3 sector_d = 0.125f * (texel_f + texel_g + texel_k + texel_l) / 4.f;
    vec3 sector_e = 0.125f * (texel_g + texel_h + texel_l + texel_m) / 4.f;

    sector_a *= karis_average(sector_a);
    sector_b *= karis_average(sector_b);
    sector_c *= karis_average(sector_c);
    sector_d *= karis_average(sector_d);
    sector_e *= karis_average(sector_e);

    return sector_a + sector_b + sector_c + sector_d + sector_e;
}

void main()
{
    // The prefilter pass is the same as the down sample passes but with karis average applied to avoid 'fire flies'.
    // https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    const vec2 texel_size = 1.f / textureSize(u_texture, 0);
    o_output = vec4(u_exposure * max(down_filter_13_tap(v_uv, texel_size), vec3(0.f)), 1.f);
}
