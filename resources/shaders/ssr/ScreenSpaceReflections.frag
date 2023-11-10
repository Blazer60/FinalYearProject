#version 460

in vec2 v_uv;

uniform sampler2D u_positionTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_colourTexture;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

uniform float u_stepSize;
uniform int u_maxStepCount;
uniform float u_thicknessThreshold;
uniform int u_binarySearchDepth;

out layout(location = 0) vec3 o_colour;

struct HitResult
{
    vec3 intersection;
    bool hit;
};

HitResult binarySearch(vec3 startPosition, vec3 endPosition, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        const vec3 midPoint = mix(startPosition, endPosition, 0.5f);
       
        vec4 positionClipSpace = u_projectionMatrix * vec4(midPoint, 1.f);
        positionClipSpace /= positionClipSpace.w;
        const vec2 currentUv = 0.5f * positionClipSpace.xy + vec2(0.5f);
        
        const vec3 samplePosition = texture(u_positionTexture, currentUv).xyz;
        const vec3 samplePositionViewSpace = (u_viewMatrix * vec4(samplePosition, 1.f)).xyz;
        const float sampleDepth = samplePositionViewSpace.z;
        const float currentDepth = midPoint.z;
        
        if (currentDepth < sampleDepth)  // We've hit.
            endPosition = midPoint;
        else
            startPosition = midPoint;
    }
    
    const vec3 midPoint = mix(startPosition, endPosition, 0.5f);
    
    vec4 positionClipSpace = u_projectionMatrix * vec4(midPoint, 1.f);
    positionClipSpace /= positionClipSpace.w;
    const vec2 currentUv = 0.5f * positionClipSpace.xy + vec2(0.5f);
    
    HitResult result;
    result.hit = true;
    result.intersection = vec3(currentUv, midPoint.z);
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
        
        if (0.f > currentUv.x || currentUv.x > 1.f)
        {
            continue;
        }
        else if (0.f > currentUv.y || currentUv.y > 1.f)
        {
            continue;
        }
        
        const vec3 samplePosition = texture(u_positionTexture, currentUv).xyz;
        const vec3 samplePositionViewSpace = (u_viewMatrix * vec4(samplePosition, 1.f)).xyz;
        const float sampleDepth = samplePositionViewSpace.z;
        const float currentDepth = position.z;
        
        const float delta = currentDepth - sampleDepth;
        if ((directionDelta.z - delta) < u_thicknessThreshold)
        {
            if (delta <= 0.f)
            {
                return binarySearch(position - directionDelta, position, u_binarySearchDepth);
                // hit
                //            HitResult result;
                //            result.hit = true;
                //            result.intersection = vec3(currentUv, currentDepth);
                //            return result;
            }
        }
        
        position += directionDelta;
    }
    
    HitResult result;
    result.hit = false;
    return result;
}

void main()
{
    const vec3 normal = normalize(texture(u_normalTexture, v_uv).rgb);
    const vec3 normalViewSpace = (u_viewMatrix * vec4(normal, 0.f)).xyz;
    
    const vec3 position = texture(u_positionTexture, v_uv).rgb;
    const vec3 positionViewSpace = (u_viewMatrix * vec4(position, 1.f)).xyz;
    
    const vec3 reflectionDirectionViewSpace = normalize(reflect(normalize(positionViewSpace), normalViewSpace));
    
    const HitResult hit = rayMarch(positionViewSpace, reflectionDirectionViewSpace, u_stepSize, u_maxStepCount);
    if (hit.hit)
        o_colour = texture(u_colourTexture, hit.intersection.xy).rgb;
    else
        o_colour = vec3(0.f);
}
