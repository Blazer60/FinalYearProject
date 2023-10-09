#version 460

in vec3 v_position;

uniform vec3 u_light_pos;
uniform float u_z_far;

void main()
{
    const float light_distance = length(v_position - u_light_pos) / u_z_far;
    gl_FragDepth = light_distance;
}
