#ifndef __LIGHT_HELPERS_HLSLI__
#define __LIGHT_HELPERS_HLSLI__

#include "Fresnel.hlsli"

// Common light
float3 ApplyLightCommon(
    float3 diffuseColor,    // Diffuse albedo
    float3 specularColor,   // Specular albedo
    float specularMask,     // Where is it shiny or dingy?
    float gloss,            // Specular power
    float3 normal,          // World-space normal
    float3 viewDir,         // World-space vector from eye to point
    float3 lightDir,        // World-space vector from point to light
    float3 lightColor       // Radiance of directional light
    )
{
    float3 halfVec = normalize(lightDir - viewDir);
    float nDotH = saturate(dot(halfVec, normal));
    float cosine = saturate(dot(lightDir, halfVec));
    
    //FSchlick(diffuseColor, specularColor, lightDir, halfVec);
    diffuseColor = FresnelSchlick(diffuseColor, 0, cosine);
    specularColor = FresnelSchlick(specularColor, 1, cosine);

    float specularFactor = specularMask * pow(nDotH, max(gloss, 1.f)) * (gloss + 2) / 8;

    float nDotL = saturate(dot(normal, lightDir));

    return nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
}

inline float3 ApplyDirectionalLight(
    float3 diffuseColor,        // Diffuse albedo
    float3 specularColor,       // Specular albedo
    float specularMask,         // Where is it shiny or dingy?
    float gloss,                // Specular power
    float3 normal,              // World-space normal
    float3 viewDir,             // World-space vector from eye to point
    float3 worldPos,            // World-space fragment position
    float3 lightDir,            // World-space vector from point to light
    float3 lightColor,          // Radiance of directional light
    float lightIntencity
    )
{
    return lightIntencity * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyConeLight(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 worldPos, // World-space fragment position
    float3 lightPos, // World-space light position
    float lightRadiusSq, // Square of light radius
    float3 lightColor, // Radiance of directional light
    float lightIntencity, // Light intencity
    float3 coneDir, // Cone direction
    float2 coneAngles // Cone angles (x: 1/cos(innter) - cos(outer), y: cos(outer))
    )
{
    float3 lightDir = lightPos - worldPos;
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;
    
    float normalizedDist = sqrt(lightDistSq) * rsqrt(lightRadiusSq);
    float distanceFalloff = 1.f / (1.0 + 25.0 * normalizedDist * normalizedDist) * saturate((1 - normalizedDist) * 5.0);

    float coneFalloff = dot(-lightDir, coneDir);
    coneFalloff = saturate((coneFalloff - coneAngles.y) * coneAngles.x);

    return lightIntencity * (coneFalloff * distanceFalloff) * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyPointLight(
    float3 diffuseColor,        // Diffuse albedo
    float3 specularColor,       // Specular albedo
    float specularMask,         // Where is it shiny or dingy?
    float gloss,                // Specular power
    float3 normal,              // World-space normal
    float3 viewDir,             // World-space vector from eye to point
    float3 lightDir,            // World-space direction of the light ray to fragment
    float lightRadiusSq,        // Square of light radius
    float3 lightColor,          // Radiance of directional light
    float lightIntencity        // Light intencity
    )
{
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;
    
    float normalizedDist = sqrt(lightDistSq) * rsqrt(lightRadiusSq);
    float distanceFalloff = 1.f / (1.0 + 25.0 * normalizedDist * normalizedDist) * saturate((1 - normalizedDist) * 5.0);
    
    return lightIntencity * distanceFalloff * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

inline float3 ApplyAmbientLight(
    float3 diffuse,         // Diffuse albedo
    float ao,               // Pre-computed ambient-occlusion
    float3 lightColor       // Radiance of ambient light
    )
{
    return ao * diffuse * lightColor;
}

#endif