#version 460

in vec2 v_uv;

uniform sampler2D u_positionTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_depthTexture;
uniform sampler2D u_colourTexture;

uniform vec3 u_cameraPositionWs;
uniform mat4 u_vpMatrix;
uniform float u_exposure;

uniform mat4 u_invProjectionMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

out layout(location = 0) vec3 o_colour;

struct Prd
{
    vec3 positionTextureSpace;
    vec3 reflectionDirectionTextureSpace;
    float maxDistance;
};

struct HitResult
{
    vec3 intersection;
    bool hit;
};

Prd myComputePositionAndReflection(vec3 normal)
{
    const vec3 position = texture(u_positionTexture, v_uv).rgb;
    const vec3 direction = normalize(u_cameraPositionWs - position);
    
    vec3 reflection = reflect(-direction, normal);
    vec4 reflectionViewSpace = u_viewMatrix * vec4(reflection, 0.f);
    o_colour = reflectionViewSpace.xyz;
    const float s = sign(reflectionViewSpace.z);
    if (reflectionViewSpace.z < 0)
        reflectionViewSpace /= reflectionViewSpace.z;
    vec4 reflectionClipSpace = u_projectionMatrix * reflectionViewSpace;
    reflectionClipSpace /= reflectionClipSpace.w;
    const vec3 reflectionTextureSpace = s * vec3(0.5f, 0.5f, 1.f) * reflectionClipSpace.xyz;
    
    const float depth = texture(u_depthTexture, v_uv).r;
    const vec3 positionTextureSpace = vec3(v_uv, depth);
    
    const vec3 R = reflectionTextureSpace;
    const vec3 P = positionTextureSpace;
    float maxDistance = R.x >= 0.f ? (1.f - P.x) / R.x : -P.x / R.x;
    maxDistance = min(maxDistance, R.y < 0.f ? -P.y / R.y : (1.f - P.y) / R.y);
    maxDistance = min(maxDistance, R.z < 0.f ? -P.z / R.z : (1.f - P.z) / R.z);
    
    Prd result;
    result.positionTextureSpace = positionTextureSpace;
    result.reflectionDirectionTextureSpace = reflectionTextureSpace;
    result.maxDistance = maxDistance;
    
    return result;
}

Prd computePositionAndReflection(vec3 sampleNormalViewSpace)
{
    const vec2 bufferSize = textureSize(u_positionTexture, 0);
    
    const float sampleDepth = texture(u_depthTexture, v_uv).r;
    vec3 samplePositionClipSpace = vec3(2.f * v_uv - vec2(1.f), sampleDepth);
    // cs.y *= -1;  // For metal api.
    
    vec4 samplePositionViewSpace = u_invProjectionMatrix * vec4(samplePositionClipSpace, 1.f);
    samplePositionViewSpace /= samplePositionViewSpace.w;
    
    const vec3 camToSampleViewSpace = normalize(samplePositionViewSpace.xyz);
    const vec3 reflectionViewSpace = reflect(camToSampleViewSpace, normalize(sampleNormalViewSpace));
    
    vec3 reflectionEndPositionViewSpace = samplePositionViewSpace.xyz + reflectionViewSpace * 1000.f;
    reflectionEndPositionViewSpace /= (reflectionEndPositionViewSpace.z < 0.f ? reflectionEndPositionViewSpace.z : 1.f);
    vec4 reflectionEndPositionClipSpace = u_projectionMatrix * vec4(reflectionEndPositionViewSpace.xyz, 1.f);
    reflectionEndPositionClipSpace /= reflectionEndPositionClipSpace.w;
    const vec3 reflectionDirection = normalize((reflectionEndPositionClipSpace.xyz - samplePositionClipSpace.xyz));
    
    const vec3 samplePositionTextureSpace = vec3(0.5f, 0.5f, 1.f) * samplePositionClipSpace + vec3(0.5f, 0.5f, 0.f);
    const vec3 reflectionDirectionTextureSpace = vec3(0.5f, 0.5f, 1.f) * reflectionDirection;
    
    const vec3 R = reflectionDirectionTextureSpace;
    const vec3 P = samplePositionTextureSpace;
    float maxDistance = R.x >= 0.f ? (1.f - P.x) / R.x : -P.x / R.x;
    maxDistance = min(maxDistance, R.y < 0.f ? -P.y / R.y : (1.f - P.y) / R.y);
    maxDistance = min(maxDistance, R.z < 0.f ? -P.z / R.z : (1.f - P.z) / R.z);
    
    Prd result;
    result.positionTextureSpace = samplePositionTextureSpace;
    result.reflectionDirectionTextureSpace = reflectionDirectionTextureSpace;
    result.maxDistance = maxDistance;
    
    return result;
}

HitResult findLinearIntersection(Prd info)
{
    const vec3 reflectionEndPosTextureSpace = info.positionTextureSpace + info.reflectionDirectionTextureSpace * info.maxDistance;
    
    // Step by the largest distance in either the x or y plane.
    vec3 deltaPosition = reflectionEndPosTextureSpace - info.positionTextureSpace;
    const vec2 bufferSize = textureSize(u_positionTexture, 0);
    ivec2 sampleScreenPosition = ivec2(info.positionTextureSpace.xy * bufferSize);
    ivec2 endPositionScreenPosition = ivec2(reflectionEndPosTextureSpace.xy * bufferSize);
    ivec2 deltaPosition2 = endPositionScreenPosition - sampleScreenPosition;
    const int maxDistance = max(abs(deltaPosition2.x), abs(deltaPosition2.y));
    deltaPosition /= maxDistance;
    
    vec3 rayPositionTextureSpace = info.positionTextureSpace + deltaPosition;
    const vec3 rayDirectionTextureSpace = deltaPosition;
    
    vec3 rayStartPosition = rayPositionTextureSpace;
    
    int hitIndex = -1;
    const int maxInteration = 5000;
    for (int i = 0; i < maxDistance && i < maxInteration; i++)
    {
        const float depth = texture(u_depthTexture, rayPositionTextureSpace.xy).r;
        const float thickness = rayPositionTextureSpace.z - depth;
        if (thickness >= 0 && thickness < 0.0001f)
        {
            hitIndex = i;
            break;
        }
        
        rayPositionTextureSpace += rayDirectionTextureSpace;
    }
    
    const bool intersected = hitIndex >= 0;
    const vec3 intersection = rayStartPosition.xyz + rayDirectionTextureSpace * float(hitIndex);
    
    HitResult result;
    result.hit = intersected;
    result.intersection = intersection;
    
    return result;
}

HitResult rayMarch(vec3 startPosition, vec3 direction, float delta, int maxIteration)
{
    const vec3 directionDelta = direction * delta;
    
    vec3 position = startPosition + directionDelta;
    
    for (int i = 0; i < maxIteration; i++)
    {
        vec4 positionClipSpace = u_projectionMatrix * vec4(position, 1.f);
        positionClipSpace /= positionClipSpace.w;
        const vec2 currentUv = 0.5f * positionClipSpace.xy + vec2(0.5f);
        
        const vec3 samplePosition = texture(u_positionTexture, currentUv).xyz;
        const vec3 samplePositionViewSpace = (u_viewMatrix * vec4(samplePosition, 1.f)).xyz;
        const float sampleDepth = samplePositionViewSpace.z;
        const float currentDepth = position.z;
        
        if (currentDepth < sampleDepth)
        {
            // hit
            HitResult result;
            result.hit = true;
            result.intersection = vec3(currentUv, currentDepth);
            return result;
        }
        
        position += directionDelta;
    }
    
    HitResult result;
    result.hit = false;
    return result;
}

void main()
{
    // todo: Do a ray marching approach in screen space. It looks like it's much easier to get visuals from it.
    const vec3 normal = normalize(texture(u_normalTexture, v_uv).rgb);
    const vec3 normalViewSpace = (u_viewMatrix * vec4(normal, 0.f)).xyz;
    
    const vec3 position = texture(u_positionTexture, v_uv).rgb;
    const vec3 positionViewSpace = (u_viewMatrix * vec4(position, 1.f)).xyz;
    
    const vec3 viewDirection = normalize(position - u_cameraPositionWs);
    const vec3 reflectionDirection = reflect(viewDirection, normal);
    const vec3 reflectionDirectionViewSpace = (u_viewMatrix * vec4(reflectionDirection, 0.f)).xyz;
//    const vec3 reflectionDirectionViewSpace = normalize(reflect(normalize(positionViewSpace), normalViewSpace));
    
    const HitResult hit = rayMarch(positionViewSpace, reflectionDirectionViewSpace, 0.1f, 30);
    if (hit.hit)
    {
//        o_colour.xy = hit.intersection.xy;
        o_colour = u_exposure * texture(u_colourTexture, hit.intersection.xy).rgb;
    }
    else
    {
        o_colour = vec3(0.f);
    }
    
//    const Prd result = computePositionAndReflection(normalViewSpace);
//    const Prd result = myComputePositionAndReflection(normal);
    
//    const HitResult hit = findLinearIntersection(result);
//    if (hit.hit)
//        o_colour = u_exposure * texture(u_colourTexture, hit.intersection.xy).rgb;
//    else
//        o_colour = vec3(0.f);
}
