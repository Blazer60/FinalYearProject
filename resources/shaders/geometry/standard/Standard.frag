#version 460

in vec2 v_uv;
in vec3 v_position_ws;
in vec3 v_normal_ws;

uniform vec3 u_ambient_colour;
uniform sampler2D u_diffuse_texture;

layout(location = 0) out vec3 o_position;
layout(location = 1) out vec3 o_normal;
layout(location = 2) out vec3 o_albedo;
layout(location = 3) out vec3 o_emissive;

void main()
{
    vec3 texture_colour = texture(u_diffuse_texture, v_uv).rgb;
    if (texture_colour == vec3(0.f))
        texture_colour = vec3(1.f);

    o_albedo = u_ambient_colour * texture_colour;
    o_position = v_position_ws;
    o_normal = v_normal_ws;
}
