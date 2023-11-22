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

float computeRoughness(vec2 hitUv)
{
    const float localRoughness = texture(u_roughnessTexture, v_uv).r;
    const vec3 position = texture(u_positionTexture, v_uv).rgb;

    const vec3 hitPosition = texture(u_positionTexture, hitUv).rgb;

    const vec3 l = normalize(hitPosition - position);  // Pretend that our reflection point is a light source direction.
    const vec3 v = normalize(u_cameraPositionWs - position);

    const float lDotV = max(0.f, dot(l, v));

    const float maxDistance = 20.f;
    const float distanceRoughness = 1.f - clamp(distance(hitPosition, position) / maxDistance, 0.f, 1.f);

    const float gloss = 1.f - localRoughness;
    const float gloss2 = gloss * gloss;


    const float alpha = gloss2 * distanceRoughness * mix(1.f, 0.f, clamp(4.f * lDotV, 0.f, 1.f));

    return 1.f - alpha;
}

// Unreal's fresnel function using spherical gaussian approximation.
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.f - roughness), f0) - f0) * pow(2.f, (-5.55473f * cosTheta - 6.98316f) * cosTheta);
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
        o_colour = vec4(0.f, 0.f, 0.f, 1.f);
        return;
    }


    const vec4 data = texture(u_reflectionDataTexture, v_uv);
    if (data.w <= 0.f)
    {
        o_colour = getSkyboxColour();
        return;
    }

    const vec2 hitUv = data.xy;

    const float alpha = computeRoughness(hitUv);
    const vec3 colour = textureLod(u_colourTexture, hitUv, alpha * (1 - u_colour_max_lod)).rgb;

    const vec3 skyboxColour = getSkyboxColour().rgb;
    o_colour = vec4(mix(skyboxColour, colour, alpha), 1.f);
}
