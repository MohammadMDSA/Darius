#include "Common.hlsli"

#ifndef __LIGHTING_HLSLI__
#define __LIGHTING_HLSLI__

#define MAX_LIGHTS 256

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 6
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 125
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 125
#endif

struct Light
{
    float3              Color;
    float3              Direction;
    float3              Position;
    float               Intencity;
    float               Radius;
    float2              SpotAngles; // x = 1.0f / (cos(coneInner) - cos(coneOuter)), y = cos(coneOuter)
    float4x4            ShadowMatrix;
    bool                CastsShadow;
    int3                padding;
};

struct Material
{
    float4              DiffuseAlbedo;
    float3              FresnelR0;
    float               Shininess;
    float               SpecularMask;
};

ByteAddressBuffer       LightMask               : register(t10);
StructuredBuffer<Light> LightData               : register(t11);
Texture2DArray<float>   lightShadowArrayTex     : register(t12);
Texture2D<float>        ssaoTexture             : register(t13);
TextureCube<float3>     radianceIBLTexture      : register(t14);
TextureCube<float3>     irradianceIBLTexture    : register(t15);

static const float3 kDielectricSpecular = float3(0.04, 0.04, 0.04);

void AntiAliasSpecular(inout float3 texNormal, inout float gloss)
{
    float normalLenSq = dot(texNormal, texNormal);
    float invNormalLen = rsqrt(normalLenSq);
    texNormal *= invNormalLen;
    float normalLen = normalLenSq * invNormalLen;
    float flatness = saturate(1 - abs(ddx(normalLen)) - abs(ddy(normalLen)));
    gloss = exp2(lerp(0, log2(gloss), flatness));
}

float FShlick(float f0, float f90, float cosine)
{
    return lerp(f0, f90, pow(1.f - cosine, 5.f));
}

float3 FShlick(float3 f0, float3 f90, float cosine)
{
    return lerp(f0, f90, pow(1.f - cosine, 5.f));
}

float3 ApplyAmbientLight(
    float3 diffuse, // Diffuse albedo
    float ao, // Pre-computed ambient-occlusion
    float3 lightColor // Radiance of ambient light
    )
{
    return ao * diffuse * lightColor;
}

float GetDirectionalShadow(uint lightIndex, float3 ShadowCoord)
{

    float3 coord = float3(ShadowCoord.x / 2 + 0.5f, -ShadowCoord.y / 2 + 0.5f, lightIndex);
    //float3 coord = float3(ShadowCoord.xy, lightIndex);
#define SINGLE_SAMPLE
#ifdef SINGLE_SAMPLE
    float result = lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord, ShadowCoord.z);
#else
    
    const float Dilation = 2.0;
    float d1 = Dilation * gShadowTexelSize.x * 0.125;
    float d2 = Dilation * gShadowTexelSize.x * 0.875;
    float d3 = Dilation * gShadowTexelSize.x * 0.625;
    float d4 = Dilation * gShadowTexelSize.x * 0.375;

    float result = (
        2.0 * lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord, ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d2, d1, 0.f), ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d1, -d2, 0.f), ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d2, -d1, 0.f), ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d1, d2, 0.f), ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d4, d3, 0.f), ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d3, -d4, 0.f), ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d4, -d3, 0.f), ShadowCoord.z) +
        lightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d3, d4, 0.f), ShadowCoord.z)
        ) / 10.0;
#endif
    return result * result;
}

float GetShadowConeLight(uint lightIndex, float3 shadowCoord)
{
    float2 scrCoord = float2(shadowCoord.x, -shadowCoord.y) / 2 + 0.5f;
    float result = lightShadowArrayTex.SampleCmpLevelZero(
        shadowSampler, float3(scrCoord, lightIndex), shadowCoord.z);
    return result * result;
}

float3 ApplyLightCommon(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 lightDir, // World-space vector from point to light
    float3 lightColor // Radiance of directional light
    )
{
    float3 halfVec = normalize(lightDir - viewDir);
    float nDotH = saturate(dot(halfVec, normal));
    float cosine = saturate(dot(lightDir, halfVec));
    
    //FSchlick(diffuseColor, specularColor, lightDir, halfVec);
    diffuseColor = FShlick(diffuseColor, 0, cosine);
    specularColor = FShlick(specularColor, 1, cosine);

    float specularFactor = specularMask * pow(nDotH, max(gloss, 1.f)) * (gloss + 2) / 8;

    float nDotL = saturate(dot(normal, lightDir));

    return nDotL * lightColor * (diffuseColor + specularFactor * specularColor);
}

float3 ApplyDirectionalLight(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 worldPos, // World-space fragment position
    float3 lightDir, // World-space vector from point to light
    float3 lightColor, // Radiance of directional light
    float lightIntencity,
    float4x4 shadowTextureMatrix,
    bool castsShadow,
	uint lightIndex
    )
{
    float shadow = 1.f;
    if (castsShadow)
    {
        float4 shadowCoord = mul(shadowTextureMatrix, float4(worldPos, 1.0));
        shadowCoord.xyz *= rcp(shadowCoord.w);
        shadow = GetDirectionalShadow(lightIndex, shadowCoord.xyz);
    }

    return shadow * lightIntencity * ApplyLightCommon(
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
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 worldPos, // World-space fragment position
    float3 lightPos, // World-space light position
    float lightRadiusSq,
    float3 lightColor, // Radiance of directional light
    float lightIntencity
    )
{
    float3 lightDir = lightPos - worldPos;
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;
    
    float normalizedDist = sqrt(lightDistSq) * rsqrt(lightRadiusSq);
    float distanceFalloff = lightIntencity / (1.0 + 25.0 * normalizedDist * normalizedDist) * saturate((1 - normalizedDist) * 5.0);
    //float distanceFalloff = saturate(lightIntencity * lightRadiusSq / (lightRadiusSq + (0.25 * lightDistSq)));

    return distanceFalloff * ApplyLightCommon(
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
    float lightRadiusSq,
    float3 lightColor, // Radiance of directional light
    float lightIntencity,
    float3 coneDir,
    float2 coneAngles
    )
{
    float3 lightDir = lightPos - worldPos;
    float lightDistSq = dot(lightDir, lightDir);
    float invLightDist = rsqrt(lightDistSq);
    lightDir *= invLightDist;
    
    float normalizedDist = sqrt(lightDistSq) * rsqrt(lightRadiusSq);
    float distanceFalloff = lightIntencity / (1.0 + 25.0 * normalizedDist * normalizedDist) * saturate((1 - normalizedDist) * 5.0);
    //float distanceFalloff = saturate(lightIntencity * lightRadiusSq / (lightRadiusSq + (0.25 * lightDistSq)));

    float coneFalloff = dot(-lightDir, coneDir);
    coneFalloff = saturate((coneFalloff - coneAngles.y) * coneAngles.x);

    return (coneFalloff * distanceFalloff) * ApplyLightCommon(
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

float3 ApplyConeShadowedLight(
    float3 diffuseColor, // Diffuse albedo
    float3 specularColor, // Specular albedo
    float specularMask, // Where is it shiny or dingy?
    float gloss, // Specular power
    float3 normal, // World-space normal
    float3 viewDir, // World-space vector from eye to point
    float3 worldPos, // World-space fragment position
    float3 lightPos, // World-space light position
    float lightRadiusSq,
    float3 lightColor, // Radiance of directional light
    float lightIntencity,
    float3 coneDir,
    float2 coneAngles,
    float4x4 shadowTextureMatrix,
    uint lightIndex,
    bool castsShadow
    )
{
    float shadow = 1.f;
    if (castsShadow)
    {
        float4 shadowCoord = mul(shadowTextureMatrix, float4(worldPos, 1.0));
        shadowCoord.xyz *= rcp(shadowCoord.w);
        shadow = GetShadowConeLight(lightIndex, shadowCoord.xyz);
    }

    return shadow * ApplyConeLight(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        worldPos,
        lightPos,
        lightRadiusSq,
        lightColor,
        lightIntencity,
        coneDir,
        coneAngles
        );
}

// Diffuse irradiance
float3 Diffuse_IBL(float3 normal, float3 toEye, float3 diffuseColor, float roughness)
{
    float LdotH = saturate(dot(normal, normalize(normal + toEye)));
    float fd90 = 0.5f + 2.f * roughness * LdotH * LdotH;
    float3 diffuseBurley = diffuseColor * FShlick(1, fd90, saturate(dot(normal, toEye)));
    return diffuseBurley * irradianceIBLTexture.Sample(defaultSampler, normal);
}

// Approximate specular IBL by sampling lower mips according to roughness.  Then modulate by Fresnel. 
float3 Specular_IBL(float3 spec, float3 normal, float3 toEye, float roughness)
{
    float lod = roughness * IBLRange + IBLBias;
    float3 specular = spec;
    specular = FShlick(specular, 1.f, saturate(dot(normal, toEye)));
    return specular * radianceIBLTexture.SampleLevel(cubeMapSampler, reflect(-toEye, normal), lod);
}

float3 ComputeLighting(
    Material mat,
    float3 pos,
    float3 normal,
    float3 toEye,
    float gloss,
    float ao)
{
    float3 result = 0.0f;
       
#define POINT_LIGHT_ARGS \
    mat.DiffuseAlbedo.rgb, \
    mat.FresnelR0, \
    mat.SpecularMask, \
    gloss, \
    normal, \
    -toEye, \
    pos, \
    light.Position, \
    lightRadiusSq, \
    light.Color, \
    light.Intencity

#define CONE_LIGHT_ARGS \
    POINT_LIGHT_ARGS, \
    light.Direction, \
    light.SpotAngles

#define SHADOWED_LIGHT_ARGS \
    CONE_LIGHT_ARGS, \
    light.ShadowMatrix, \
    i, \
    light.CastsShadow
    
    for (uint i = 0; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        uint masks = LightMask.Load((i / 32) * 4);
        uint idx = i - (i / 32) * 32;
        if (!(masks & (1u << (31 - idx))))
            continue;
        
        Light light = LightData[i];

        if (i < NUM_DIR_LIGHTS)
        {
            result += ApplyDirectionalLight(
                mat.DiffuseAlbedo.rgb,
                mat.FresnelR0,
                mat.SpecularMask,
                gloss,
                normal,
                -toEye,
                pos,
                -light.Direction,
                light.Color,
                light.Intencity,
                light.ShadowMatrix,
                true,
                i);
            continue;
        }
            
        float3 lightDir = light.Position - pos;
        float lightDistSq = dot(lightDir, lightDir);
        float lightRadiusSq = light.Radius * light.Radius;
        
        // If pixel position is too far from light
        if (lightDistSq >= lightRadiusSq)
            continue;
        
        if (i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS)
            result += ApplyPointLight(POINT_LIGHT_ARGS);
        else
            //result += ApplyConeLight(CONE_LIGHT_ARGS);
            result += ApplyConeShadowedLight(SHADOWED_LIGHT_ARGS);
        
    }
    result += ApplyAmbientLight(mat.DiffuseAlbedo.rgb, ao, gAmbientLight.rgb);
    return result;
}

float3 ComputeLitColor(
    float3 worldPos,
    uint2 screenPos,
    float3 normal,
    float3 toEye,
    float4 diffuseAlbedo,
    float metallic,
    float roughness,
    float3 emissive,
    float ao,
    float specularMask,
    float3 F0)
{
    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, F0, shininess, specularMask };

    float gloss = shininess * 256;
    
    float ssao = ssaoTexture[screenPos];
    
    ao *= ssao;
    
    // TODO: Add specular anti-aliasing
    float3 directLight = ComputeLighting(mat, worldPos, normal, toEye, gloss, ao);

    float3 c_diff = diffuseAlbedo.rgb * (1 - kDielectricSpecular) * (1 - metallic) * ao;
    float3 c_spec = lerp(kDielectricSpecular, diffuseAlbedo.rgb, metallic) * ao;
    
    float3 litColor = emissive + directLight;

    return litColor + Diffuse_IBL(normal, toEye, c_diff, roughness) + Specular_IBL(c_spec, normal, toEye, roughness);
}

#endif