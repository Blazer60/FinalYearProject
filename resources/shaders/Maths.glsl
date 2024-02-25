#version 460 core

const float PI = 3.14159265359f;

vec3 fresnelSchlick(vec3 f0, float nDotL)
{
    return f0 + (1.f - f0) * pow(1 - nDotL, 5);
}

// (Cigolle, Z. H, et al., 2014) A Survey of efficient representations for Independent Unit Vectors.
vec2 signNotZero(vec2 v)
{
    return vec2((v.x >= 0.f) ? 1.f : -1.f, (v.y >= 0.f) ? 1.f : -1.f);
}
// end.

vec2 uvToNdc(vec2 uv)
{
    return 2.f * uv - vec2(1.f);
}

vec3 uvToNdc(vec3 uv)
{
    return 2.f * uv - vec3(1.f);
}

float saturate(float p)
{
    return clamp(p, 0.f, 1.f);
}

// Typically used for returning a vector in the light's direction.
vec3 cosImportanceSample(vec2 rand, vec3 n)
{
    const float phi = 2.f * PI * rand.x;
    const float sinTheta = sqrt(rand.y);
    const float cosTheta = sqrt(1.f - rand.y);

    const vec3 randomDirection = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    const vec3 up = abs(n.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
    const vec3 tangent = normalize(cross(up, n));
    const vec3 biTangent = cross(n, tangent);

    const vec3 sampleVec = tangent * randomDirection.x + biTangent * randomDirection.y + n * randomDirection.z;
    return normalize(sampleVec);
}
