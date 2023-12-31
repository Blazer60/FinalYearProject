#version 460

layout(location=0) in vec3 a_position;

uniform mat4 u_mvp_matrix;

out vec2 v_uv;

void main()
{
    const vec4 position_cs = u_mvp_matrix * vec4(a_position, 1.f);
    gl_Position = position_cs;
    v_uv = 0.5f * ((position_cs.xy / position_cs.w) + vec2(1.f));
}
