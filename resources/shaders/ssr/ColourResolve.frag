#version 460

in vec2 v_uv;

uniform sampler2D u_albedoTexture;
uniform sampler2D u_positionTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_roughnessTexture;
uniform sampler2D u_metallicTexture;
uniform sampler2D u_reflectionDataTexture;
uniform sampler2D u_colourTexture;
uniform sampler2D u_depthTexture;
uniform samplerCube u_irradianceTexture;
uniform samplerCube u_pre_filterTexture;
uniform sampler2D u_brdfLutTexture;

uniform vec3 u_cameraPositionWs;
uniform int u_colour_max_lod;
uniform float u_luminance_multiplier;

out layout(location = 0) vec4 o_colour;

const float PI = 3.14159265359f;

float computeRoughness(vec2 hitUv)
{
    const float localRoughness = texture(u_roughnessTexture, v_uv).r;
    const vec3 position = texture(u_positionTexture, v_uv).rgb;

    const vec3 hitPosition = texture(u_positionTexture, hitUv).rgb;

    const vec3 l = normalize(hitPosition - position);  // Pretend that our reflection point is a light source direction.
    const vec3 v = normalize(u_cameraPositionWs - position);

    const float lDotV = max(0.f, dot(l, v));

    const float maxDistance = 20.f;  // todo: This should be a uniform.
    const float distanceRoughness = 1.f - clamp(distance(hitPosition, position) / maxDistance, 0.f, 1.f);

    const float gloss = 1.f - localRoughness;
    const float gloss2 = gloss * gloss;


    const float alpha = gloss2 * gloss2 * distanceRoughness * mix(1.f, 0.f, clamp(4.f * lDotV, 0.f, 1.f));

    return 1.f - alpha;
}

// Unreal's fresnel function using spherical gaussian approximation.
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.f - roughness), f0) - f0) * pow(2.f, (-5.55473f * cosTheta - 6.98316f) * cosTheta);
}

// Unreal's fresnel function using spherical gaussian approximation.
vec3 fresnelSchlick(float vDotH, vec3 f0)
{
    return f0 + (1.f - f0) * pow(2.f, (-5.55473f * vDotH - 6.98316f) * vDotH);
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

// Unreal's geometry schlick function.
float geometrySchlick(float vDotN, float k)
{
    return vDotN / (vDotN * (1.f - k) + k);
}

// Unreal's geometry smith function. Note the remapping of roughness before squaring
// should only be done on analytical lights. Remap function: (R + 1) / 2
float geometrySmith(float vDotN, float lDotN, float roughness)
{
    const float r = roughness + 1.f;
    const float k = r * r / 8.f;
    return geometrySchlick(vDotN, k) * geometrySchlick(lDotN, k);
}

vec3 brdf(vec2 hitUv)
{
    // Calculate all of the random variables we need.
    const vec3 albedo = texture(u_albedoTexture, v_uv).rgb;
    const vec3 position = texture(u_positionTexture, v_uv).rgb;
    const float roughness = texture(u_roughnessTexture, v_uv).r;
    const float metallic = texture(u_metallicTexture, v_uv).r;

    const vec3 hitPosition = texture(u_positionTexture, hitUv).rgb;
    const vec3 l = normalize(hitPosition - position);
    const vec3 n = normalize(texture(u_normalTexture, v_uv).rgb);
    const vec3 v = normalize(u_cameraPositionWs - position);
    const vec3 h = normalize(l + v);

    const float vDotN = max(dot(v, n), 0.f);
    const float lDotN = max(dot(l, n), 0.f);
    const float vDotH = max(dot(v, h), 0.f);
    const float nDotH = max(dot(n, h), 0.f);

    const vec3 f0 = mix(vec3(0.04f), albedo, metallic);

    // Calculate how the material interacts with the light.
    const vec3 fresnel = fresnelSchlick(vDotH, f0);
    const float distribution = distributionGgx(nDotH, roughness);
    const float geometry = geometrySmith(vDotN, lDotN, roughness);

    const vec3 specular = distribution * geometry * fresnel / (4.f * vDotN * lDotN + 0.0001f);

    const vec3 kD = (vec3(1.f) - fresnel) * (1.f - metallic);
    const vec3 diffuse = kD * albedo / PI;

    const vec3 irradiance = (diffuse + specular) * vDotH;
    return irradiance;
}

vec4 colourResolve()
{
    vec2[] offsets = {
        vec2(-1.f,  0.f),
        vec2( 1.f,  0.f),
        vec2( 0.f, -1.f),
        vec2( 0.f,  1.f),
    };

    vec2 bufferSize = textureSize(u_reflectionDataTexture, 0);
    vec2 texelSize = 1.f / bufferSize;

    vec3 result = vec3(0.f);
    vec3 weightSum = vec3(0.0001f);
    float totalAlpha = 0.f;

    for (int i = 0; i < 4; ++i)
    {
        const vec2 pixelOffset = offsets[i] * texelSize;
        const vec4 data = texture(u_reflectionDataTexture, v_uv + pixelOffset * 0.5f);

        if (data.w <= 0.f)
        {
            totalAlpha += 1.f;
            continue;
        }

        const vec2 hitUv = data.xy;
        const float pdf = data.z;

        const float alpha = computeRoughness(hitUv);
        totalAlpha += alpha;

        const vec3 weight = brdf(hitUv) / pdf;
        result += textureLod(u_colourTexture, hitUv, alpha * (1 - u_colour_max_lod)).rgb * weight;
        weightSum += weight;
    }

    result /= weightSum;
    totalAlpha *= 0.25;

    return vec4(result, totalAlpha);
}

vec4 getSkyboxColour()
{
    const vec3 albedo   = texture(u_albedoTexture, v_uv).rgb;
    const vec3 position = texture(u_positionTexture, v_uv).rgb;
    const float metallic = texture(u_metallicTexture, v_uv).r;
    const float roughness = texture(u_roughnessTexture, v_uv).r;

    const vec3 n = normalize(texture(u_normalTexture, v_uv).rgb);
    const vec3 v = normalize(u_cameraPositionWs - position);
    const vec3 r = reflect(-v, n);

    const float vDotN = max(dot(v, n), 0.f);

    const vec3 f0 = mix(vec3(0.04f), albedo, metallic);

    const vec3 fresnel = fresnelSchlickRoughness(vDotN, f0, roughness);

    const vec3 kD = (vec3(1.f) - fresnel) * (1.f - metallic);

    const vec3 irradiance = texture(u_irradianceTexture, n).rgb;
    const vec3 diffuse = kD * irradiance * albedo;

    const float maxReflectionLod = 4.f;
    const vec3 preFilterColour = textureLod(u_pre_filterTexture, r, roughness * maxReflectionLod).rgb;
    const vec2 brdf = texture(u_brdfLutTexture, vec2(vDotN, roughness)).rg;
    const vec3 specular = preFilterColour * (fresnel * brdf.x + brdf.y);

    return vec4((specular + diffuse) * u_luminance_multiplier, roughness);
}

void main()
{
    if (texture(u_depthTexture, v_uv).r >= 1.f)
    {
        o_colour = getSkyboxColour();
        return;
    }

    const vec4 colour = colourResolve();
    const vec3 skyboxColour = getSkyboxColour().rgb;
    o_colour = vec4(mix(colour.rgb, skyboxColour, colour.a), colour.a);
}
