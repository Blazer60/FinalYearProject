#version 460

in vec2 v_uv;

uniform samplerCube u_environment_texture;
uniform mat4 u_view_matrix;

out layout(location = 0) vec3 o_colour;

const float PI = 3.14159265359f;

void main()
{
    const vec3 direction = vec3(u_view_matrix * vec4(2.f * v_uv.x - 1.f, 2.f * v_uv.y - 1.f, -1.f, 1.f));
    const vec3 normal = normalize(direction);

    vec3 irradiance = vec3(0.f);

    vec3 up = vec3(0.f, 1.f, 0.f);
    const vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    const float sampleDelta = 0.025f;
    float sampleCount = 0.f;
    for (float phi = 0.f; phi < 2.f * PI; phi += sampleDelta)
    {
        for (float theta = 0.f; theta < 0.5f * PI; theta += sampleDelta)
        {
            // Spherical to cartesian (in tangent space).
            const vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // Tangent to world
            const vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += min(texture(u_environment_texture, sampleVec).rgb, 100.f) * cos(theta) * sin(theta);
            sampleCount++;
        }
    }

    irradiance = (PI / float(sampleCount)) * irradiance;

    o_colour = irradiance;
}