#version 460

in vec2 v_uv;

uniform sampler2D u_positionTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_depthTexture;
uniform sampler2D u_colourTexture;

uniform vec3 u_cameraPositionWs;
uniform mat4 u_vpMatrix;
uniform float u_exposure;

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

Prd computePositionAndReflection()
{
    const vec3 positionWs = texture(u_positionTexture, v_uv).rgb;
    vec4 positionClipSpace = u_vpMatrix * vec4(positionWs, 1.f);
    positionClipSpace.xyz /= positionClipSpace.w;

    const vec3 viewDirection = normalize(u_cameraPositionWs - positionWs);
    const vec3 normal = normalize(texture(u_normalTexture, v_uv)).rgb;
    const vec3 reflectionDirection = reflect(viewDirection, normal);

    // ... Some weird way of computing the reflection direction here ...

    vec4 reflectionDirectionClipSpace = u_vpMatrix * vec4(reflectionDirection, 0.f);
//    reflectionDirectionClipSpace /= reflectionDirectionClipSpace.w;

    const vec3 positionTextureSpace = 0.5f * positionClipSpace.xyz + vec3(0.5f);
    const vec3 reflectionDirectionTextureSpace = 0.5f * reflectionDirectionClipSpace.xyz;
    
    o_colour = abs(reflectionDirectionTextureSpace);

    // Computing the maximum distance the ray will travel by checking the bounds in texture coordinates.
    float maxDistance = reflectionDirectionTextureSpace.x >= 0.f ? (1.f - positionTextureSpace.x) / reflectionDirectionTextureSpace.x : -positionTextureSpace.x / reflectionDirectionTextureSpace.x;
    maxDistance = min(maxDistance, reflectionDirectionTextureSpace.y < 0 ? -positionTextureSpace.y / reflectionDirectionTextureSpace.y : (1.f - positionTextureSpace.y) / reflectionDirectionTextureSpace.y);
    maxDistance = min(maxDistance, reflectionDirectionTextureSpace.z < 0 ? -positionTextureSpace.z / reflectionDirectionTextureSpace.z : (1.f - positionTextureSpace.z) / reflectionDirectionTextureSpace.z);

    Prd result;
    result.positionTextureSpace = positionTextureSpace;
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
        if (thickness >= 0)
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

void main()
{
    const Prd result = computePositionAndReflection();
    const HitResult hit = findLinearIntersection(result);
    if (hit.hit)
    {
        o_colour = texture(u_colourTexture, hit.intersection.xy).rgb;
    }
    else
    {
        o_colour = vec3(0.f);
    }
    o_colour = result.reflectionDirectionTextureSpace;
}
