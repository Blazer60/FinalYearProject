#version 460

in vec2 v_uv;

uniform samplerCube u_environment_texture;
uniform mat4 u_view_matrix;
uniform float u_roughness;

out layout(location = 0) vec3 o_colour;

const float PI = 3.14159265359;

float radicalInverseVdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radicalInverseVdc(i));
}

vec3 importanceSampleGgx(vec2 xI, vec3 N, float roughness)
{
    const float a = roughness * roughness;

    const float phi = 2.f * PI * xI.x;
    const float cosTheta = sqrt((1.f - xI.y) / (1.f + (a * a - 1.f) * xI.y));
    const float sinTheta = sqrt(1.f - cosTheta * cosTheta);

    const vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    const vec3 up = abs(N.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
    const vec3 tangent = normalize(cross(up, N));
    const vec3 biTangent = cross(N, tangent);

    const vec3 sampleVec = tangent * H.x + biTangent * H.y + N * H.z;
    return normalize(sampleVec);
}

void main()
{
    const vec3 direction = vec3(u_view_matrix * vec4(2.f * v_uv.x - 1.f, 2.f * v_uv.y - 1.f, -1.f, 1.f));
    const vec3 normal = normalize(direction);

    const vec3 N = normal;
    const vec3 R = N;
    const vec3 V = R;

    const uint sampleCount = 1024u;
    float totalWeight = 0.f;
    vec3 prefilteredColour = vec3(0.f);

    for (uint i = 0u; i < sampleCount; ++i)
    {
        const vec2 xI = hammersley(i, sampleCount);
        const vec3 H = importanceSampleGgx(xI, N, u_roughness);
        const vec3 L = normalize(2.f * dot(V, H) * H - V);

        const float nDotL = max(dot(N, L), 0.f);
        if (nDotL > 0.f)
        {
            prefilteredColour += min(texture(u_environment_texture, L).rgb, vec3(50.f)) * nDotL;
            totalWeight += nDotL;
        }
    }

    prefilteredColour = prefilteredColour / totalWeight;

    o_colour = prefilteredColour;
}