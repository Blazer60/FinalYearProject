#version 460

in vec2 v_uv;

uniform sampler2D u_texture;
uniform mat4 u_view_matrix;

out layout(location = 0) vec3 o_colour;

vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    const vec2 invAtan = vec2(0.1591f, 0.3183f);
    uv *= invAtan;
    uv += 0.5f;
    return uv;
}

void main()
{
    const vec3 direction = vec3(u_view_matrix * vec4(2.f * v_uv.x - 1.f, 2.f * v_uv.y - 1.f, -1.f, 1.f));
    const vec2 uv = sampleSphericalMap(normalize(direction));
    o_colour = texture(u_texture, uv).rgb;
}