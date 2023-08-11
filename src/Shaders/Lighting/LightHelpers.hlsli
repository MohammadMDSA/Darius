#ifndef __LIGHT_HELPERS_HLSLI__
#define __LIGHT_HELPERS_HLSLI__

#include "../Utils/Fresnel.hlsli"
#include "../Utils/BxDF.hlsli"

static const float3 kDielectricSpecular = float3(0.04, 0.04, 0.04);

// Common light
inline float3 ApplyLightCommon(
    float3 diffuseColor,    // Diffuse albedo
    float3 specularColor,   // Specular albedo
    float specularMask,     // Where is it shiny or dingy?
    float roughness,        // Specular power
    float3 normal,          // World-space normal
    float3 viewDir,         // World-space vector from eye to point
    float3 lightDir,        // World-space vector from point to light
    float3 lightColor       // Radiance of directional light
    )
{
    
    // Legacy Shading
#if 0
    float3 halfVec = normalize(lightDir - viewDir);
    float nDotH = saturate(dot(halfVec, normal));
    float cosine = saturate(dot(lightDir, halfVec));
    
    diffuseColor = FresnelSchlick(diffuseColor, 0, cosine);
    specularColor = FresnelSchlick(specularColor, 1, cosine);
    float gloss = (1 - roughness) * 255;
    float specularFactor = specularMask * pow(nDotH, max(gloss, 1.f)) * (gloss + 2) / 8;

    float nDotL = saturate(dot(normal, lightDir));

    return nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
#else
    // GGX specular and Hammon Diffuse
    float3 result = BxDF::DirectLighting::Shade(
            MaterialType::Default,
            diffuseColor,
            specularColor,
            specularMask,
            lightColor,
            roughness,
            normal,
            -viewDir,
            lightDir);    
            
    return result;
#endif
}

inline float3 ApplyDirectionalLight(
    float3 diffuseColor,        // Diffuse albedo
    float3 specularColor,       // Specular albedo
    float specularMask,         // Where is it shiny or dingy?
    float roughness,            // Specular power
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
        roughness,
        normal,
        viewDir,
        lightDir,
        lightColor
        );
}

float3 ApplyConeLight(
    float3 diffuseColor,        // Diffuse albedo
    float3 specularColor,       // Specular albedo
    float specularMask,         // Where is it shiny or dingy?
    float roughness,            // Specular power
    float3 normal,              // World-space normal
    float3 viewDir,             // World-space vector from eye to point
    float3 worldPos,            // World-space fragment position
    float3 lightPos,            // World-space light position
    float lightRadiusSq,        // Square of light radius
    float3 lightColor,          // Radiance of directional light
    float lightIntencity,       // Light intencity
    float3 coneDir,             // Cone direction
    float2 coneAngles           // Cone angles (x: 1/cos(innter) - cos(outer), y: cos(outer))
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
        roughness,
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
    float roughness,            // Specular power
    float3 normal,              // World-space normal
    float3 viewDir,             // World-space vector from eye to point
    float3 toLight,             // World-space vector from fragment to light source
    float lightRadiusSq,        // Square of light radius
    float3 lightColor,          // Radiance of directional light
    float lightIntencity        // Light intencity
    )
{
    float lightDistSq = dot(toLight, toLight);
    float invLightDist = rsqrt(lightDistSq);
    float3 lightDir = toLight * invLightDist;
    
    float normalizedDist = sqrt(lightDistSq) * rsqrt(lightRadiusSq);
    float distanceFalloff = 1.f / (1.0 + 25.0 * normalizedDist * normalizedDist) * saturate((1 - normalizedDist) * 5.0);
    
    return lightIntencity * distanceFalloff * ApplyLightCommon(
        diffuseColor,
        specularColor,
        specularMask,
        roughness,
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