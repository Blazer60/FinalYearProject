#version 460

in vec2 v_uv;

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

float geometrySchlickGgx(float nDotV, float roughness)
{
    const float r = roughness;
    const float k = r * r / 2.f;

    return nDotV / (nDotV * (1.f - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    const float nDotV = max(dot(N, V), 0.f);
    const float nDotL = max(dot(N, L), 0.f);
    const float ggx2 = geometrySchlickGgx(nDotV, roughness);
    const float ggx1 = geometrySchlickGgx(nDotL, roughness);

    return ggx1 * ggx2;
}

vec2 integrateBrdf(float nDotV, float roughness)
{
    const vec3 V = vec3(sqrt(1.f - nDotV * nDotV), 0.f, nDotV);

    float A = 0.f;
    float B = 0.f;

    vec3 N = vec3(0.f, 0.f, 1.f);

    const uint sampleCount = 1024u;
    for (uint i = 0u; i < sampleCount; ++i)
    {
        const vec2 xI = hammersley(i, sampleCount);
        const vec3 H = importanceSampleGgx(xI, N, roughness);
        const vec3 L = normalize(2.f * dot(V, H) * H - V);

        const float nDotL = max(L.z, 0.f);
        const float nDotH = max(H.z, 0.f);
        const float vDotH = max(dot(V, H), 0.f);

        if (nDotL > 0.f)
        {
            const float G = geometrySmith(N, V, L, roughness);
            const float gVis = (G * vDotH) / (nDotH * nDotV);
            const float fC = pow(1.f - vDotH, 5.f);

            A += (1.f - fC) * gVis;
            B += fC * gVis;
        }
    }

    A /= float(sampleCount);
    B /= float(sampleCount);

    return vec2(A, B);
}

void main()
{
    o_colour.rg = integrateBrdf(v_uv.x, v_uv.y);
}