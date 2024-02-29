#version 460

#include "../GBuffer.glsl"
#include "../../Colour.glsl"

in vec2 v_uv;
in vec3 v_position_ws;
in vec3 v_normal_ws;
in mat3 v_tbn_matrix;
in vec3 v_camera_position_ts;
in vec3 v_position_ts;
in vec2 vScreenUv;

uniform vec3 u_ambient_colour;
uniform sampler2D u_diffuse_texture;
uniform vec3 specularColour;
uniform sampler2D specularTexture;
uniform sampler2D u_normal_texture;
uniform sampler2D u_height_texture;
uniform sampler2D u_roughness_texture;
uniform sampler2D u_metallic_texture;
uniform vec3 u_emissive_colour;
uniform mat4 u_model_matrix;

uniform float u_height_scale = 0.1f;
uniform int u_min_height_samples = 4;
uniform int u_max_height_samples = 32;

uniform float u_roughness;
uniform float u_metallic;

vec2 height_map(vec2 uv)
{
    const vec3 view_direction = normalize(v_camera_position_ts - v_position_ts);

    const float view_dot = clamp(dot(vec3(0.f, 0.f, 1.f), view_direction), 0.f, 1.f);
    const int samples = int(ceil(mix(u_min_height_samples, u_max_height_samples, view_dot)));

    const float layer_delta = 1.f / float(samples);
    const vec2 uv_delta = view_direction.xy * u_height_scale / float(samples);

    vec2 current_uv = uv;
    float current_layer_depth = 0.f;
    float current_height = 1.f - texture(u_height_texture, current_uv).r;

    vec2 previous_uv = uv;
    float previous_layer_depth = current_layer_depth;
    float previous_height = current_height;

    while(current_layer_depth < current_height)
    {
        previous_height = current_height;
        previous_uv = current_uv;
        previous_layer_depth = current_layer_depth;

        current_uv -= uv_delta;
        current_height = 1.f - texture(u_height_texture, current_uv).r;

        current_layer_depth += layer_delta;
    }

    // No delta between height samples so we can assume that the surface is flat. (stops div by zero error)
    const float error = 0.001f;
    if (current_height - previous_height <= error)
        return current_uv;

    // How close the samples are to the view line.
    const float previous_delta = previous_height - previous_layer_depth;
    const float current_delta = current_height - current_layer_depth;

    const float weight = current_delta / (current_delta / previous_delta);
    const vec2 out_uv = mix(current_uv, previous_uv, weight);

    return out_uv;
}

void main()
{
    const vec2 uv = height_map(v_uv);
    vec3 texture_colour = texture(u_diffuse_texture, uv).rgb;
    vec3 model_normal_direction = 2.f * texture(u_normal_texture, uv).rgb - vec3(1.f);

    float o_roughness = texture(u_roughness_texture, uv).r;
    if (o_roughness <= 0.f)
        o_roughness = u_roughness;
    o_roughness = max(0.02f, o_roughness);

    float o_metallic = texture(u_metallic_texture, uv).r;
    if (o_metallic <= 0.f)
        o_metallic = u_metallic;

    if (texture_colour == vec3(0.f))
        texture_colour = vec3(1.f);

    vec3 o_normal = vec3(0.f);
    if (model_normal_direction == vec3(-1.f))
        o_normal = v_normal_ws;
    else
        o_normal = normalize(v_tbn_matrix * model_normal_direction);

    vec3 o_albedo = sRgbToLinear(u_ambient_colour * texture_colour);
    vec3 o_position = v_position_ws;
    vec3 o_emissive = u_emissive_colour;

    vec3 specularTextureColour = texture(specularTexture, uv).rgb;
    if (specularTextureColour == vec3(0.f))
        specularTextureColour = vec3(1.f);

    GBuffer gBuffer;
    gBuffer.emissive = o_emissive;
    gBuffer.diffuse = o_albedo;
    gBuffer.specular = sRgbToLinear(specularColour * specularTextureColour);
    gBuffer.normal = o_normal;
    gBuffer.roughness = o_roughness;

    pushToStorageGBuffer(gBuffer, ivec2(0));
}
