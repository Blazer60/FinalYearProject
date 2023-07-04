#version 460

in vec2 v_uv;
in vec3 v_position_ws;
in vec3 v_normal_ws;
in mat3 v_tbn_matrix;

uniform vec3 u_ambient_colour;
uniform sampler2D u_diffuse_texture;
uniform sampler2D u_normal_texture;
uniform mat4 u_model_matrix;

layout(location = 0) out vec3 o_position;
layout(location = 1) out vec3 o_normal;
layout(location = 2) out vec3 o_albedo;
layout(location = 3) out vec3 o_emissive;
//layout(location = 4) out float o_depth;

void main()
{
    vec3 texture_colour = texture(u_diffuse_texture, v_uv).rgb;
    vec3 model_normal_direction = 2.f * texture(u_normal_texture, v_uv).rgb - vec3(1.f);

    if (texture_colour == vec3(0.f))
        texture_colour = vec3(1.f);

    if (model_normal_direction == vec3(-1.f))
        o_normal = v_normal_ws;
    else
        o_normal = normalize(v_tbn_matrix * model_normal_direction);

    o_albedo = u_ambient_colour * texture_colour;
    o_position = v_position_ws;
//    o_depth = gl_FragCoord.z;
}
