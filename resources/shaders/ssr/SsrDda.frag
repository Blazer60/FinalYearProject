#version 460

in vec2 v_uv;

uniform sampler2D u_positionTexture;
uniform sampler2D u_normalTexture;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform vec3 u_cameraPosition;
uniform mat4 u_proj;

uniform float u_nearPlaneZ;
uniform float u_farPlaneZ;

uniform float u_exposure;
uniform sampler2D u_colourTexture;

out layout(location = 0) vec3 o_colour;

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
    float stride, float maxSteps, float maxDistance,
    out vec2 hitPixel, out vec3 csHitPoint)
{
    // Clip to the near plane.
    const float rayLength = ((csOrigin.z + csDirection.z * maxDistance) > u_nearPlaneZ) ? (u_nearPlaneZ - csOrigin.z) / csDirection.z : maxDistance;
    const vec3 csEndPoint = csOrigin + csDirection * rayLength;
    hitPixel = vec2(-1.f);
    
    // Project into screen space.
    const vec4 H0 = u_proj * vec4(csOrigin, 1.f);
    const vec4 H1 = u_proj * vec4(csEndPoint, 1.f);
    const float k0 = 1.f / H0.w;
    const float k1 = 1.f / H1.w;
    const vec3 Q0 = csOrigin * k0;
    const vec3 Q1 = csEndPoint * k1;
    
    // Screen-space endpoints.
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;
    
    // [Optional Clip function here if wanted (probs just set the border colour to be invalid)].
    
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
    // Jitter stuff goes here.
    float prevZMaxEstimate = csOrigin.z;
    
    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, K from K0 to K1
    vec3 Q = Q0;
    float k = k0;
    float stepCount = 0.f;
    float end = P1.x * stepDirection;
    for(vec2 P = P0;
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
//        const float sceneZMax = mix(0.f, 1.f, (u_viewMatrix * vec4(positionWorldSpace, 1.f)).z / u_farPlaneZ);
        const float sceneZMax = (u_viewMatrix * vec4(positionWorldSpace, 1.f)).z;
        const float sceneZMin = sceneZMax - zThickness;
        
        if (((rayZMax >= sceneZMin) && (rayZMin <= sceneZMax)) || sceneZMax == 0.f)
            break;
        
        // Increment everything that is suppose to be inside the for loop section.
        Q.z += dQ.z;
        k += dK;
        stepCount += 1.f;
    }
    
    const vec2 csZBufferSize = textureSize(u_positionTexture, 0);
    
    // Advance Q based on the number of steps.
    Q.xy += dQ.xy * stepCount;
    csHitPoint = Q * (1.f / k);
    return all(lessThanEqual(abs(hitPixel - (csZBufferSize * 0.5f)), csZBufferSize * 0.5f));
}

void main()
{
    const vec3 position = texture(u_positionTexture, v_uv).xyz;
    const vec3 normal = normalize(texture(u_normalTexture, v_uv).xyz);
    
    const vec3 direction = normalize(u_cameraPosition - position);
    const vec3 reflection = reflect(-direction, normal);
    
    const vec3 reflectionViewSpace = (u_viewMatrix * vec4(reflection, 0.f)).xyz;
    const vec3 positionViewSpace = (u_viewMatrix * vec4(position, 1.f)).xyz;
    
    const float zThickness = 1.f;  // 1-1000
    const float stride = 1.f;  // 1-4
    const float maxSteps = 500.f;
    const float maxDistance = 1000.f;
    vec2 hitPixel;
    vec3 hitPointViewSpace;
    
//    traceScreenSpaceRay(positionViewSpace, reflectionViewSpace, zThickness, stride, maxSteps, maxDistance, hitPixel, hitPointViewSpace);
    
    if (traceScreenSpaceRay(positionViewSpace, reflectionViewSpace, zThickness, stride, maxSteps, maxDistance, hitPixel, hitPointViewSpace))
        o_colour = u_exposure * texelFetch(u_colourTexture, ivec2(hitPixel), 0).rgb;
    else
        o_colour = vec3(0.f);
}