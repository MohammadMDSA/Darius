#ifndef __LIGHTING_HLSLI__
#define __LIGHTING_HLSLI__

#include "Common.hlsli"

#include "../../Lighting/LightBindings.hlsli"
#include "../../Lighting/LightHelpers.hlsli"
#include "../../Utils/Fresnel.hlsli"

struct Material
{
    float4              DiffuseAlbedo;
    float3              FresnelR0;
    float               Roughness;
    float               SpecularMask;
};

Texture2DArray<float>   DirectioanalightShadowArrayTex      : register(t12);
TextureCubeArray<float> PointLightShadowArrayTex            : register(t13);
Texture2DArray<float>   SpotLightShadowArrayTex             : register(t14);
Texture2D<float>        ssaoTexture                         : register(t15);
TextureCube<float3>     radianceIBLTexture                  : register(t16);
TextureCube<float3>     irradianceIBLTexture                : register(t17);

void AntiAliasSpecular(inout float3 texNormal, inout float gloss)
{
    float normalLenSq = dot(texNormal, texNormal);
    float invNormalLen = rsqrt(normalLenSq);
    texNormal *= invNormalLen;
    float normalLen = normalLenSq * invNormalLen;
    float flatness = saturate(1 - abs(ddx(normalLen)) - abs(ddy(normalLen)));
    gloss = exp2(lerp(0, log2(gloss), flatness));
}

float GetDirectionalShadow(uint lightIndex, float3 ShadowCoord)
{
    float3 coord = float3(ShadowCoord.xy, lightIndex);
//#define SINGLE_SAMPLE
#ifdef SINGLE_SAMPLE
    float result = DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord, ShadowCoord.z);
#else
    
    const float Dilation = 2.0;
    float d1 = Dilation * gShadowTexelSize.x * 0.125;
    float d2 = Dilation * gShadowTexelSize.x * 0.875;
    float d3 = Dilation * gShadowTexelSize.x * 0.625;
    float d4 = Dilation * gShadowTexelSize.x * 0.375;

    float result = (
        2.0 * DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord, ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d2, d1, 0.f), ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d1, -d2, 0.f), ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d2, -d1, 0.f), ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d1, d2, 0.f), ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d4, d3, 0.f), ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d3, -d4, 0.f), ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d4, -d3, 0.f), ShadowCoord.z) +
        DirectioanalightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d3, d4, 0.f), ShadowCoord.z)
        ) / 10.0;
#endif
    return result * result;
}

float GetShadowConeLight(uint lightIndex, float3 shadowCoord, float3 wPos)
{
    float2 scrCoord = float2(shadowCoord.x, -shadowCoord.y) / 2 + 0.5f;
//#define SINGLE_SAMPLE
#ifdef SINGLE_SAMPLE
    float result = SpotLightShadowArrayTex.SampleCmpLevelZero(
        shadowSampler, float3(scrCoord.xy, lightIndex - NUM_POINT_LIGHTS - NUM_DIR_LIGHTS), shadowCoord.z);
#else

    const float Dilation = 2.0;
    float d1 = Dilation * gShadowTexelSize.x * 0.125;
    float d2 = Dilation * gShadowTexelSize.x * 0.875;
    float d3 = Dilation * gShadowTexelSize.x * 0.625;
    float d4 = Dilation * gShadowTexelSize.x * 0.375;
    float3 coord = float3(scrCoord, lightIndex - NUM_POINT_LIGHTS - NUM_DIR_LIGHTS);
            
    float result = (
        2.0 * SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord, shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d2, d1, 0.f), shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d1, -d2, 0.f), shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d2, -d1, 0.f), shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d1, d2, 0.f), shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d4, d3, 0.f), shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(-d3, -d4, 0.f), shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d4, -d3, 0.f), shadowCoord.z) +
        SpotLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float3(d3, d4, 0.f), shadowCoord.z)
        ) / 10.0;
#endif
    return result * result;
}

float GetShadowPointLight(uint lightIndex, float3 lightToPos)
{
    float4x4 shadow = g_LightData[lightIndex].ShadowMatrix;
    
    float3 absToPixel = abs(lightToPos);
    float Z = -max(absToPixel.z, max(absToPixel.x, absToPixel.y));
    float2 params = float2(shadow[2][2], shadow[2][3]);
    float depth = (params.y + params.x * Z) / (-Z);
    
    lightToPos = normalize(lightToPos);
//#define SINGLE_SAMPLE
#ifdef SINGLE_SAMPLE      
    float result = PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, float4(-lightToPos.x, lightToPos.y, lightToPos.z, lightIndex - NUM_DIR_LIGHTS), depth);
#else
    const float Dilation = 2.0;
    float d1 = Dilation * gShadowTexelSize.x * 0.125;
    float d2 = Dilation * gShadowTexelSize.x * 0.875;
    float d3 = Dilation * gShadowTexelSize.x * 0.625;
    float d4 = Dilation * gShadowTexelSize.x * 0.375;
    float4 coord = float4(-lightToPos.x, lightToPos.y, lightToPos.z, lightIndex - NUM_DIR_LIGHTS);
    float result = (
        2.0 * PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord, depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(-d2, d1, 0.f, 0.f), depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(-d1, -d2, 0.f, 0.f), depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(d2, -d1, 0.f, 0.f), depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(d1, d2, 0.f, 0.f), depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(-d4, d3, 0.f, 0.f), depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(-d3, -d4, 0.f, 0.f), depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(d4, -d3, 0.f, 0.f), depth) +
        PointLightShadowArrayTex.SampleCmpLevelZero(shadowSampler, coord + float4(d3, d4, 0.f, 0.f), depth)
    ) / 10.0;
#endif
    return result * result;
}

float3 ApplyDirectionalShadowedLight(
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

    return shadow * ApplyDirectionalLight(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        worldPos,
        lightDir,
        lightColor,
        lightIntencity
        );
}

float3 ApplyPointShadowedLight(
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
    uint lightIndex,
    bool castsShadow
    )
{
    float3 toLight = lightPos - worldPos;
    
    float shadow = 1.f;
    
    if(castsShadow)
        shadow = GetShadowPointLight(lightIndex, -toLight);
    
    return shadow * ApplyPointLight(
        diffuseColor,
        specularColor,
        specularMask,
        gloss,
        normal,
        viewDir,
        toLight,
        lightRadiusSq,
        lightColor,
        lightIntencity
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
    uint lightIndex,
    float3 coneDir,
    float2 coneAngles,
    float4x4 shadowTextureMatrix,
    bool castsShadow
    )
{
    float shadow = 1.f;
    if (castsShadow)
    {
        float4 shadowCoord = mul(shadowTextureMatrix, float4(worldPos, 1.0));
        shadowCoord.xyz *= rcp(shadowCoord.w);
        shadow = GetShadowConeLight(lightIndex, shadowCoord.xyz, worldPos);
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
    float3 diffuseBurley = diffuseColor * FresnelSchlick(1, fd90, saturate(dot(normal, toEye)));
    return diffuseBurley * irradianceIBLTexture.Sample(defaultSampler, normal);
}

// Approximate specular IBL by sampling lower mips according to roughness.  Then modulate by Fresnel. 
float3 Specular_IBL(float3 spec, float3 normal, float3 toEye, float roughness)
{
    float lod = roughness * IBLRange + IBLBias;
    float3 specular = spec;
    specular = FresnelSchlick(specular, 1.f, saturate(dot(normal, toEye)));
    return specular * radianceIBLTexture.SampleLevel(cubeMapSampler, reflect(-toEye, normal), lod);
}

float3 ComputeLighting(
    Material mat,
    float3 pos,
    float3 normal,
    float3 toEye,
    float ao)
{
    float3 result = 0.0f;
       
#define POINT_LIGHT_ARGS \
    mat.DiffuseAlbedo.rgb, \
    mat.FresnelR0, \
    mat.SpecularMask, \
    mat.Roughness, \
    normal, \
    -toEye, \
    pos, \
    light.Position, \
    lightRadiusSq, \
    light.Color, \
    light.Intencity, \
    i

#define CONE_LIGHT_ARGS \
    POINT_LIGHT_ARGS, \
    light.Direction, \
    light.SpotAngles

#define SHADOWED_LIGHT_ARGS \
    CONE_LIGHT_ARGS, \
    light.ShadowMatrix, \
    light.CastsShadow
    
    for (uint i = 0; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        uint masks = g_LightMask.Load((i / 32) * 4);
        uint idx = i - (i / 32) * 32;
        if (!(masks & (1u << (31 - idx))))
            continue;
        
        Light light = g_LightData[i];

        if (i < NUM_DIR_LIGHTS)
        {
            result += ApplyDirectionalShadowedLight(
                mat.DiffuseAlbedo.rgb,
                mat.FresnelR0,
                mat.SpecularMask,
                mat.Roughness,
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
            result += ApplyPointShadowedLight(POINT_LIGHT_ARGS, true);
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
    
    Material mat = { diffuseAlbedo, F0, roughness, specularMask };
    
    float ssao = ssaoTexture[screenPos];
    
    ao *= ssao;
    
    // TODO: Add specular anti-aliasing
    float3 directLight = ComputeLighting(mat, worldPos, normal, toEye, ao);

    float3 c_diff = diffuseAlbedo.rgb * (1 - kDielectricSpecular) * (1 - metallic) * ao;
    float3 c_spec = lerp(kDielectricSpecular, diffuseAlbedo.rgb, metallic) * ao;
    
    float3 litColor = emissive + directLight;

    float3 diffIbl = Diffuse_IBL(normal, toEye, c_diff, roughness);
    float3 specIbl = Specular_IBL(c_spec, normal, toEye, roughness);
    return litColor + diffIbl + specIbl;
}

#endif