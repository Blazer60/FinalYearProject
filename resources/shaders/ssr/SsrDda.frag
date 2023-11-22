#version 460

in vec2 v_uv;

uniform sampler2D u_positionTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_depthTexture;
uniform sampler2D u_roughnessTexture;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform vec3 u_cameraPosition;
uniform mat4 u_proj;

uniform float u_nearPlaneZ;
uniform float u_farPlaneZ;

uniform float u_exposure;
uniform sampler2D u_colourTexture;

out layout(location = 0) vec4 o_colour;

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

void swap(inout float a, inout float b)
{
    const float temp = a;
    a = b;
    b = temp;
}

float distanceSquared(vec2 a, vec2 b)
{
    const vec2 x = b - a;
    return dot(x, x);
}

bool traceScreenSpaceRay(
    vec3 csOrigin, vec3 csDirection, float zThickness,
    float stride, float maxSteps, float maxDistance, float jitter,
    out vec2 hitPixel, out vec3 csHitPoint)
{
    const vec2 csZBufferSize = textureSize(u_positionTexture, 0);
    
    // Clip to the near plane.
    const float rayLength = ((csOrigin.z + csDirection.z * maxDistance) > u_nearPlaneZ) ? (u_nearPlaneZ - csOrigin.z) / csDirection.z : maxDistance;
    const vec3 csEndPoint = csOrigin + csDirection * rayLength;
    hitPixel = vec2(-1.f);
    
    // Project into screen space.
    const vec4 H0 = u_proj * vec4(csOrigin, 1.f);
    const vec4 H1 = u_proj * vec4(csEndPoint, 1.f);
    float k0 = 1.f / H0.w;
    float k1 = 1.f / H1.w;
    vec3 Q0 = csOrigin * k0;
    vec3 Q1 = csEndPoint * k1;
    
    // Screen-space endpoints.
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;
    
    // [Optional Clip function here if wanted (probs just set the border colour to be invalid)].
    const float xMax = csZBufferSize.x - 0.5f;
    const float xMin = 0.5f;
    const float yMax = csZBufferSize.y - 0.5f;
    const float yMin = 0.5f;
    float alpha = 0.f;
    // Assume P0 is in the viewport (P1 - P0 is never zero when clipping).
    if ((P1.y > yMax) || (P1.y < yMin))
        alpha = (P1.y - ((P1.y > yMax) ? yMax : yMin)) / (P1.y - P0.y);
    if ((P1.x > xMax) || (P1.x < xMin))
        alpha = max(alpha, (P1.x - ((P1.x > xMax) ? xMax : xMin)) / (P1.x - P0.x));
    P1 = mix(P1, P0, alpha);
    k1 = mix(k1, k0, alpha);
    Q1 = mix(Q1, Q0, alpha);
    // [End Optional Clip function]
    
    P1 += vec2((distanceSquared(P0, P1) < 0.0001f) ? 0.01f : 0.f);
    vec2 delta = P1 - P0;
    bool permute = false;
    if (abs(delta.x) < abs(delta.y))
    {
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }
    
    const float stepDirection = sign(delta.x);
    const float invdx = stepDirection / delta.x;  // Inverse delta x?
    
    // Track the derivatives of Q and K.
    vec3 dQ = (Q1 - Q0) * invdx;
    float dK = (k1 - k0) * invdx;
    vec2 dP = vec2(stepDirection, delta.y * invdx);
    
    dP *= stride;
    dQ *= stride;
    dK *= stride;
    P0 += dP * jitter;
    Q0 += dQ * jitter;
    k0 += dK * jitter;
    float prevZMaxEstimate = csOrigin.z;
    
    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, K from K0 to K1
    vec3 Q = Q0;
    float k = k0;
    float stepCount = 0.f;
    float end = P1.x * stepDirection;
    vec2 P = P0;
    for(;
        ((P.x * stepDirection) <= end) && (stepCount < maxSteps);
        P += dP)
    {
        // Project back from homogeneous to camera space.
        hitPixel = permute ? P.yx : P;
        
        // The depth range that the ray covers within this loop iteration
        // assumes that they ray is mving in increasing z and swap if backwards.
        float rayZMin = prevZMaxEstimate;
        // Compute the value at 1/2 pixel into the future.
        float rayZMax = (dQ.z * 0.5f + Q.z) / (dK * 0.5f + k);
        prevZMaxEstimate = rayZMax;
        if (rayZMin > rayZMax)
            swap(rayZMin, rayZMax);
        
        // Camera-space z.
        const vec3 positionWorldSpace = texelFetch(u_positionTexture, ivec2(hitPixel), 0).xyz;
        const float sceneZMax = (u_viewMatrix * vec4(positionWorldSpace, 1.f)).z;
        const float sceneZMin = sceneZMax - zThickness;
        
        if (((rayZMax >= sceneZMin) && (rayZMin <= sceneZMax)) || sceneZMax == 0.f)
            break;
        
        // Increment everything that is suppose to be inside the for loop section.
        Q.z += dQ.z;
        k += dK;
        stepCount += 1.f;
    }
    
    
    // Advance Q based on the number of steps.
    Q.xy += dQ.xy * stepCount;
    csHitPoint = Q * (1.f / k);
    return all(lessThanEqual(abs(hitPixel - (csZBufferSize * 0.5f)), csZBufferSize * 0.5f)) && stepCount < maxSteps && (P.x * stepDirection <= end);
}

// Unreal's and Disney's distribution function.
float distributionGgx(float nDotH, float roughness)
{
    const float alpha = roughness * roughness;
    const float alpha2 = alpha * alpha;
    const float nDotH2 = nDotH * nDotH;
    const float denominator = nDotH2 * (alpha2 - 1.f) + 1.f;

    return alpha2 / (PI * denominator * denominator);
}

void main()
{
    const float depth = texture(u_depthTexture, v_uv).r;
    if (depth >= 1.f)
    {
        o_colour = vec4(0.f);
        discard;  // We've missed all geometry so don't do anything.
    }
    
    const vec3 position = texture(u_positionTexture, v_uv).xyz;
    const vec3 normal = normalize(texture(u_normalTexture, v_uv).xyz);
    const float roughness = texture(u_roughnessTexture, v_uv).r;
    
    const vec3 direction = normalize(u_cameraPosition - position);
    
    const vec2 csZBufferSize = textureSize(u_positionTexture, 0);

    const uint sampleCount = 1u;
    for (uint i = 0; i < sampleCount; i++)
    {
        const vec2 xI = hammersley(i, sampleCount);
        const vec3 H = importanceSampleGgx(xI, normal, roughness);
        const vec3 reflection = reflect(-direction, H);  // aka. L

        const float nDotH = dot(normal, H);
        const float vDotH = dot(direction, H);
        const float D = distributionGgx(nDotH, roughness);

        // This is the probability distribution function for both unreal and frostbite.
        const float pdf = max(0.0001f, (D * nDotH) / (4.f * vDotH));

        const vec3 reflectionViewSpace = (u_viewMatrix * vec4(reflection, 0.f)).xyz;
        // Slight offset to avoid self-intersection (just like ray tracing).
        const vec3 positionViewSpace = (u_viewMatrix * vec4(position + normal * 0.1f, 1.f)).xyz;
        
        const float zThickness = 0.1f;  // (0-inf] Controls the 'smearing'.
        const float stride = 1.f;  // [1-4]
        const float maxSteps = 1500.f;
        const float maxDistance = 1000.f;
        const float jitter = 0.f; // [0, 1]
        vec2 hitPixel;
        vec3 hitPointViewSpace;
        
        if (traceScreenSpaceRay(positionViewSpace, reflectionViewSpace, zThickness, stride, maxSteps, maxDistance, jitter, hitPixel, hitPointViewSpace))
        {
            const vec2 hitUv = hitPixel / csZBufferSize;
            const float depthTest = texture(u_depthTexture, hitUv).r;
            if (depthTest >= 1.f)  // Check if we've just hit the sky. Not the best way to do this.
                o_colour = vec4(0.f);
            else
                o_colour = vec4(hitUv.xy, pdf, 1.f);
        }
        else
            o_colour = vec4(0.f);
    }
}