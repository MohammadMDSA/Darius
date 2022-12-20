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
    float4 Color;
    float4 Direction;
    float4 Position;
    float FalloffStart;
    float FalloffEnd;
    float SpotPower;
};

struct Material
{
    float4  DiffuseAlbedo;
    float3  FresnelR0;
    float   Shininess;
};

ByteAddressBuffer LightMask                 : register(t10);
StructuredBuffer<Light> LightData           : register(t11);
TextureCube<float3> radianceIBLTexture      : register(t12);
TextureCube<float3> irradianceIBLTexture    : register(t13);

static const float3 kDielectricSpecular = float3(0.04, 0.04, 0.04);

float Pow5(float x)
{
    float xSq = x * x;
    return xSq * xSq * x;
}
float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// Schlick gives any approximation to Fresnel reflection
// R0 = ( (n - 1) / (n + 1) ) ^ 2, where n is the index of refraction.
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));
    
    float f0 = 1.f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.f - R0) * Pow5(f0);
    
    return reflectPercent;
}

// Shlick's approximation of Fresnel
float3 Fresnel_Shlick(float3 F0, float3 F90, float cosine)
{
    return lerp(F0, F90, Pow5(1.0 - cosine));
}

float3 BlinnPhong(
    float3 lightStrength,
    float3 lightVec,
    float3 normal,
    float3 toEye,
    Material mat)
{
    // Derive m from the shininess, which is derived from the roughness.
    const float m = mat.Shininess * 256.f;
    float3 halfVec = normalize(toEye + lightVec);
    
    float lambCos = saturate(dot(halfVec, normal));
    float roughnessFactor = lambCos == 0 ? 0.f : (m + 2.f) * pow(lambCos, m) / 8.f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);
    
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    
    // Our spec formula goes outside [0, 1] range, but we are doing
    // LDR rendering. so scaale it down a bit.
    
    specAlbedo = specAlbedo / (specAlbedo + 1.f);
    
    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for directional lights.
//---------------------------------------------------------------------------------------
float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction.xyz;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Color.rgb * ndotl;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for point lights.
//---------------------------------------------------------------------------------------
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 result;

    // The vector from the surface to the light.
    float3 lightVec = L.Position.xyz - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > L.FalloffEnd)
        result = float3(0.f, 0.f, 0.f);
    else
    {
        // Normalize the light vector.
        lightVec /= d;

        // Scale light down by Lambert's cosine law.
        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.Color.rgb * ndotl;

        // Attenuate light by distance.
        float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
        lightStrength *= att;

        result = BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }

    return result;
}

//---------------------------------------------------------------------------------------
// Evaluates the lighting equation for spot lights.
//---------------------------------------------------------------------------------------
float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 result;

    // The vector from the surface to the light.
    float3 lightVec = L.Position.xyz - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range testz
    if (d > L.FalloffEnd)
        result = float3(0.f, 0.f, 0.f);
    else
    {
        // Normalize the light vector.
        lightVec /= d;

        // Scale light down by Lambert's cosine law.
        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.Color.rgb * ndotl;

        // Attenuate light by distance.
        float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
        lightStrength *= att;

        // Scale by spotlight
        float spotFactor = pow(max(dot(-lightVec, L.Direction.xyz), 0.0f), L.SpotPower);
        lightStrength *= spotFactor;

        result = BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }

    return result;
}

// Diffuse irradiance
float3 Diffuse_IBL(float3 normal, float3 toEye, float3 diffuseColor, float roughness)
{
    float LdotH = saturate(dot(normal, normalize(normal + toEye)));
    float fd90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
    float3 DiffuseBurley = diffuseColor * Fresnel_Shlick(1.f, fd90, saturate(dot(normal, toEye)));
    return DiffuseBurley * irradianceIBLTexture.Sample(defaultSampler, normal);
}

// Approximate specular IBL by sampling lower mips according to roughness.  Then modulate by Fresnel. 
float3 Specular_IBL(float3 spec, float3 normal, float3 toEye, float roughness)
{
    float lod = roughness * IBLRange + IBLBias;
    float3 specular = Fresnel_Shlick(spec, 1, saturate(dot(normal, toEye)));
    return specular * radianceIBLTexture.SampleLevel(cubeMapSampler, reflect(-toEye, normal), lod);
}

float3 ComputeLighting(
    Material mat,
    float3 pos,
    float3 normal,
    float3 toEye,
    float3 shadowFactor)
{
    float3 result = 0.0f;

    uint i = 0;

    for (i = 0; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        uint masks = LightMask.Load((i / 32) * 4);
        uint idx = i - (i / 32) * 32;
        if (!(masks & (1 << (31 - idx))))
            continue;
        
        if (i < NUM_DIR_LIGHTS)
            result += shadowFactor * ComputeDirectionalLight(LightData[i], mat, normal, toEye);
        else if (i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS)
            result += ComputePointLight(LightData[i], mat, pos, normal, toEye);
        else
            result += ComputeSpotLight(LightData[i], mat, pos, normal, toEye);
        
    }
    
    return result;
}

float3 ComputeLitColor(float3 worldPos, float3 normal, float3 toEye, float4 diffuseAlbedo, float metallic, float roughness, float3 emissive, float occlusion, float3 F0)
{
    const float shininess = 1.0f - roughness;
    Material mat = { diffuseAlbedo, F0, shininess };
    float3 shadowFactor = 1.0f;

    float3 directLight = ComputeLighting(mat, worldPos,
        normal, toEye, shadowFactor);

    float3 c_diff = diffuseAlbedo.rgb * (1 - kDielectricSpecular) * (1 - metallic) * occlusion;
    float3 c_spec = lerp(kDielectricSpecular, diffuseAlbedo.rgb, metallic) * occlusion;
    
    float3 litColor = emissive + directLight + (gAmbientLight.rgb * diffuseAlbedo.rgb);

    return litColor + Diffuse_IBL(normal, toEye, c_diff, roughness) + Specular_IBL(c_spec, normal, toEye, roughness);
}

#endif