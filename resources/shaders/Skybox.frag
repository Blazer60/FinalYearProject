#version 460

in vec2 v_uv;

uniform samplerCube u_skybox_texture;
// It's not quite the vp matrix as the view matrix should not include the position offset of the camera.
uniform mat4 u_inverse_vp_matrix;

out vec3 o_colour;

void main()
{
    const vec2 uv_normalised = 2.f * v_uv - vec2(1.f);

    vec4 near_plane = vec4(uv_normalised.x, uv_normalised.y, -1.0f, 1.f);
    vec4 far_plane  = vec4(uv_normalised.x, uv_normalised.y,  1.0f, 1.f);

    near_plane = u_inverse_vp_matrix * near_plane;
    far_plane  = u_inverse_vp_matrix * far_plane;

    near_plane /= near_plane.w;
    far_plane  /= far_plane.w;

    const vec3 direction = (far_plane - near_plane).xyz;
    const vec3 colour = texture(u_skybox_texture, direction.xyz).rgb;
    o_colour = colour;
}