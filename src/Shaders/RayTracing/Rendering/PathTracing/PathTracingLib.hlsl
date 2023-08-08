#define HLSL

#include "../../RaytracingHlslCompat.h"
#include "../../Material.hlsli"
#include "BxDF.hlsli"
#include "../../RayTracingCommon.hlsli"
#include "../../Utils/RayTracingUtils.hlsli"
#include "../../../Utils/PixelPacking.hlsli"
#include "../../../Utils/ShaderUtility.hlsli"

// Common (static) samplers
SamplerState        g_DefaultSampler    : register(s10);
SamplerState        g_CubeMapSampler    : register(s11);
SamplerState        g_LinearWrapSampler : register(s12);

// GLOBALS /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

cbuffer g_CB : register(b0, space0)
{
    float4x4            g_View;
    float4x4            g_InvView;
    float4x4            g_Proj;
    float4x4            g_InvProj;
    float4x4            g_ViewProj;
    float4x4            g_InvViewProj;
    float4x4            g_InvViewProjEyeCenter;
    float4              g_FrustumPlanes[6];
    float4              g_ShadowTexelSize;
    float3              g_CameraPosW;
    float               c_bPerObjectPad1;
    float2              g_RenderTargetSize;
    float2              g_InvRenderTargetSize;
    float               g_NearZ;
    float               g_FarZ;
    float               g_TotalTime;
    float               g_DeltaTime;
    float4              g_AmbientLight;
    float               g_IBLRange;
    float               g_IBLBias;
    
};
        
cbuffer g_RTCB : register(b1, space0)
{
    uint                g_MaxRadianceRayRecursionDepth;
}

RaytracingAccelerationStructure     g_RtScene                   : register(t0, space0);
TextureCube<float3>                 g_RadianceIBLTexture        : register(t1, space0);
TextureCube<float3>                 g_IrradianceIBLTexture      : register(t2, space0);


// Locals //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// RayGen
RWTexture2D<float4>                 l_ColorOutput               : register(u0, space1);
RWTexture2D<float4>                 l_GBufferPosition           : register(u1, space1);
RWTexture2D<NormalDepthTexFormat>   l_GBufferNormalDepth        : register(u2, space1);
RWTexture2D<float>                  l_GBufferDepth              : register(u3, space1);

// Hit Group    
StructuredBuffer<uint16_t>          l_Indices                   : register(t10, space2);
StructuredBuffer<RTVertexPositionNormalTangentTexture> l_Vertices : register(t11, space2);  
cbuffer                             l_MeshConstants             : register(b0, space2)
{
    float4x4                        l_world;
    float3x3                        l_worldIT;
}
// Includes Material data


namespace PathtracerRayType {
    enum Enum {
        Radiance = 0,	// ~ Radiance ray generating color and GBuffer data
        Shadow,         // ~ Shadow/visibility rays

        Count
    };
}
        
namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0;   // Everything is visible.
    namespace HitGroup {
        static const UINT Offset[PathtracerRayType::Count] =
        {
            0, // Radiance ray
            1, // Shadow ray
        };
		static const UINT GeometryStride = PathtracerRayType::Count;
    }
    namespace MissShader {
        static const UINT Offset[PathtracerRayType::Count] =
        {
            0, // Radiance ray
            1, // Shadow ray
        };
    }
}
        
PathTracerRayPayload TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth, float tMin = NEAR_PLANE, float tMax = FAR_PLANE, float bounceContribution = 1, bool cullNonOpaque = false)
{
    PathTracerRayPayload rayPayload;
    rayPayload.RayRecursionDepth = currentRayRecursionDepth + 1u;
    rayPayload.Radiance = 0;
    rayPayload.GBuffer.THit = HitDistanceOnMiss;
    rayPayload.GBuffer.HitPosition = 0;
    rayPayload.GBuffer.DiffuseByte3 = 0;
    rayPayload.GBuffer.EncodedNormal = 0;

    if (currentRayRecursionDepth >= g_MaxRadianceRayRecursionDepth)
    {
        rayPayload.Radiance = float3(133, 161, 179) / 255.0;
        return rayPayload;
    }

    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.Origin;
    rayDesc.Direction = ray.Direction;
    rayDesc.TMin = tMin;
    rayDesc.TMax = tMax;

    UINT rayFlags = (cullNonOpaque ? RAY_FLAG_CULL_NON_OPAQUE : 0);
    rayFlags |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

	TraceRay(g_RtScene,
        rayFlags,
		TraceRayParameters::InstanceMask,
		TraceRayParameters::HitGroup::Offset[PathtracerRayType::Radiance],
		TraceRayParameters::HitGroup::GeometryStride,
		TraceRayParameters::MissShader::Offset[PathtracerRayType::Radiance],
		rayDesc, rayPayload);

	return rayPayload;
}

// Returns radiance of the traced refracted ray.
float3 TraceRefractedGBufferRay(in float3 hitPosition, in float3 wt, in float3 N, in float3 objectNormal, inout PathTracerRayPayload rayPayload, in float TMax = 10000)
{
    // Here we offset ray start along the ray direction instead of surface normal 
    // so that the reflected ray projects to the same screen pixel. 
    // Offsetting by surface normal would result in incorrect mappating in temporally accumulated buffer. 
    float tOffset = 0.001f;
    float3 offsetAlongRay = tOffset * wt;

    float3 adjustedHitPosition = hitPosition + offsetAlongRay;

    Ray ray = { adjustedHitPosition,  wt };

    float tMin = 0.f; 
    float tMax = TMax; 

    // TRADEOFF: Performance vs visual quality
    // Cull transparent surfaces when casting a transmission ray for a transparent surface.
    // Spaceship in particular has multiple layer glass causing a substantial perf hit 
    // with multiple bounces along the way.
    // This can cause visual pop ins however, such as in a case of looking at the spaceship's
    // glass cockpit through a window in the house. The cockpit will be skipped in this case.
    bool cullNonOpaque = true;

    rayPayload = TraceRadianceRay(ray, rayPayload.RayRecursionDepth, tMin, tMax, 0, cullNonOpaque);

    if (rayPayload.GBuffer.THit != HitDistanceOnMiss)
    {
        // Add current thit and the added offset to the thit of the traced ray.
        rayPayload.GBuffer.THit += RayTCurrent() + tOffset;
    }

    return rayPayload.Radiance;
}
  
// Returns radiance of the traced reflected ray.
float3 TraceReflectedGBufferRay(in float3 hitPosition, in float3 wi, in float3 N, in float3 objectNormal, inout PathTracerRayPayload rayPayload, in float TMax = 10000)
{
    // Here we offset ray start along the ray direction instead of surface normal 
    // so that the reflected ray projects to the same screen pixel. 
    // Offsetting by surface normal would result in incorrect mappating in temporally accumulated buffer. 
    float tOffset = 0.001f;
    float3 offsetAlongRay = tOffset * wi;

    float3 adjustedHitPosition = hitPosition + offsetAlongRay;

    Ray ray = { adjustedHitPosition,  wi };

    float tMin = 0.f; 
    float tMax = TMax;

    rayPayload = TraceRadianceRay(ray, rayPayload.RayRecursionDepth, tMin, tMax);
    if (rayPayload.GBuffer.THit != HitDistanceOnMiss)
    {
        // Add current thit and the added offset to the thit of the traced ray.
        rayPayload.GBuffer.THit += RayTCurrent() + tOffset;
    }

    return rayPayload.Radiance;
}
// Update GBuffer with the hit that has the largest diffuse component.
// Prioritize larger diffuse component hits as it is a direct scale of the AO contribution to the final color value.
// This doesn't always result in the largest AO contribution as the final color contribution depends on the AO coefficient as well,
// but this is the best estimate at this stage.
void UpdateGBufferOnLargerDiffuseComponent(inout PathTracerRayPayload rayPayload, in PathTracerRayPayload _rayPayload, in float3 diffuseScale)
{
    float3 diffuse = Byte3ToNormalizedFloat3(rayPayload.GBuffer.DiffuseByte3);

    // Adjust the diffuse by the diffuse scale, i.e. BRDF value of the returned ray.
    float3 _diffuse = Byte3ToNormalizedFloat3(_rayPayload.GBuffer.DiffuseByte3) * diffuseScale;
    
    if (_rayPayload.GBuffer.THit != HitDistanceOnMiss && RGBToLuminance(diffuse) < RGBToLuminance(_diffuse))
    {
        rayPayload.GBuffer = _rayPayload.GBuffer;
        rayPayload.GBuffer.DiffuseByte3 = NormalizedFloat3ToByte3(_diffuse);
    }
}
          
float3 Shade(
    inout PathTracerRayPayload rayPayload,
    in float3 N,
    in float3 objectNormal,
    in float3 hitPosition,
    in PrimitiveMaterialBuffer material)
{
    float3 V = -WorldRayDirection();
    float pdf;
    float3 indirectContribution = 0;
    float3 L = 0;

    const float3 Kd = material.Albedo;
    const float3 Ks = material.Specular;
    const float3 Kr = material.Metallic;
    const float3 Kt = material.Transmissivity;
    const float roughness = material.Rroughness;

     // Direct illumination
    rayPayload.GBuffer.DiffuseByte3 = NormalizedFloat3ToByte3(Kd);
    if (!BxDF::IsBlack(material.Albedo) || !BxDF::IsBlack(material.Specular))
    {
        float3 lightPos = float3(100.f, 100.f, 100.f);
        float3 wi = normalize(lightPos - hitPosition);

        // Raytraced shadows.
        bool isInShadow = false; //TraceShadowRayAndReportIfHit(hitPosition, wi, N, rayPayload);

        L += BxDF::Shade(
            material.Type,
            Kd,
            Ks,
            float3(1.f, 1.f, 1.f), // Light color
            isInShadow,
            g_AmbientLight.xyz,
            roughness,
            N,
            V,
            wi);
    }

    //L += g_AmbientLight.xyz * Kd; // Applying ambient light
    L += material.Emission;

    // Specular Indirect Illumination
    bool isReflective = !BxDF::IsBlack(Kr);
    bool isTransmissive = !BxDF::IsBlack(Kt);

    // Handle cases where ray is coming from behind due to imprecision,
    // don't cast reflection rays in that case.
    float smallValue = 1e-6f;
    //isReflective = dot(V, N) > smallValue ? isReflective : false;

    if (isReflective || isTransmissive)
    {
        //if (isReflective 
        //    && (BxDF::Specular::Reflection::IsTotalInternalReflection(V, N) 
        //        || material.Type == MaterialType::Mirror))
        //{
        //    PathTracerRayPayload reflectedRayPayLoad = rayPayload;
        //    float3 wi = reflect(-V, N);
                
        //    L += Kr * TraceReflectedGBufferRay(hitPosition, wi, N, objectNormal, reflectedRayPayLoad);
        //    UpdateGBufferOnLargerDiffuseComponent(rayPayload, reflectedRayPayLoad, Kr);
        //}
        //else // No total internal reflection
        //{
            float3 Fo = Ks;
            if (isReflective)
            {
                // Radiance contribution from reflection.
                float3 wi;
                float3 Fr = Kr * BxDF::Specular::Reflection::Sample_Fr(V, wi, N, Fo);    // Calculates wi
                
                PathTracerRayPayload reflectedRayPayLoad = rayPayload;
                // Ref: eq 24.4, [Ray-tracing from the Ground Up]
                L += Fr * TraceReflectedGBufferRay(hitPosition, wi, N, objectNormal, reflectedRayPayLoad);
                UpdateGBufferOnLargerDiffuseComponent(rayPayload, reflectedRayPayLoad, Fr);
            }

            if (isTransmissive)
            {
                // Radiance contribution from refraction.
                float3 wt;
                float3 Ft = Kt * BxDF::Specular::Transmission::Sample_Ft(V, wt, N, Fo);    // Calculates wt

                PathTracerRayPayload refractedRayPayLoad = rayPayload;

                L += Ft * TraceRefractedGBufferRay(hitPosition, wt, N, objectNormal, refractedRayPayLoad);
                UpdateGBufferOnLargerDiffuseComponent(rayPayload, refractedRayPayLoad, Ft);
            }
        //}
    }

    return L;
}
            
[shader("raygeneration")]
void MainRenderRayGen()
{
    uint2 DTid = DispatchRaysIndex().xy;

	// Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    Ray ray = GenerateCameraRay(DTid, g_CameraPosW, g_InvViewProjEyeCenter);

	// Cast a ray into the scene and retrieve GBuffer information.
    UINT currentRayRecursionDepth = 0;
    PathTracerRayPayload rayPayload = TraceRadianceRay(ray, currentRayRecursionDepth);

    // Invalidate perfect mirror reflections that missed. 
    // There is no We don't need to calculate AO for those.
    bool hasNonZeroDiffuse = rayPayload.GBuffer.DiffuseByte3 != 0;
    rayPayload.GBuffer.THit = hasNonZeroDiffuse ? rayPayload.GBuffer.THit: HitDistanceOnMiss;
    bool hasCameraRayHitGeometry = rayPayload.GBuffer.THit != HitDistanceOnMiss;

	// Write out GBuffer information to rendertargets.
    l_GBufferPosition[DTid] = float4(rayPayload.GBuffer.HitPosition, 1);

    float rayLength = HitDistanceOnMiss;
    if (hasCameraRayHitGeometry)
    {
        rayLength = rayPayload.GBuffer.THit;
            
        // Calculate linear z-depth
        float3 cameraDirection = GenerateForwardCameraRayDirection(g_InvViewProjEyeCenter);
        float linearDepth = rayLength * dot(ray.Direction, cameraDirection);

        //l_GBufferNormalDepth[DTid] = EncodeNormalDepth(DecodeNormal(rayPayload.GBuffer.EncodedNormal), linearDepth);
        //l_GBufferDepth[DTid] = linearDepth;
    }
    else // No geometry hit.
    {
        l_GBufferNormalDepth[DTid] = 0;
        l_GBufferDepth[DTid] = 0;
    }

    l_ColorOutput[DTid] = float4(rayPayload.Radiance, 1);
}
    
float3 NormalMap(
    in float3 normal,
    in float2 texCoord,
    in RTVertexPositionNormalTangentTexture vertices[3],
    in BuiltInTriangleIntersectionAttributes attr)
{
    float3 vertexTangents[3] = { vertices[0].Tangent.xyz, vertices[1].Tangent.xyz, vertices[2].Tangent.xyz };
    float3 tangent = HitAttribute(vertexTangents, attr);
    

    float3 texSample = l_TexNormal.SampleLevel(l_NormalSampler, texCoord, 0).xyz;
    float3 bumpNormal = normalize(texSample * 2.f - 1.f);
    return BumpMapNormalToWorldSpaceNormal(bumpNormal, normal, tangent);
}
    
[shader("miss")]
void MainRenderMiss(inout PathTracerRayPayload payload)
{
    payload.Radiance = g_RadianceIBLTexture.SampleLevel(g_CubeMapSampler, WorldRayDirection(), 0);
}

static const float3 kDielectricSpecular = float3(0.04, 0.04, 0.04);
            
[shader("closesthit")]
void MainRenderCHS(inout PathTracerRayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint startIndex = PrimitiveIndex() * 3;
    const uint3 indices = { l_Indices[startIndex], l_Indices[startIndex + 1], l_Indices[startIndex + 2] };

    // Retrieve vertices for the hit triangle.
    RTVertexPositionNormalTangentTexture vertices[3] =
    {
        l_Vertices[indices[0]],
        l_Vertices[indices[1]],
        l_Vertices[indices[2]]
    };

    float2 vertexTexCoords[3] = { vertices[0].Tex, vertices[1].Tex, vertices[2].Tex };
    float2 texCoord = HitAttribute(vertexTexCoords, attr);

    PrimitiveMaterialBuffer material;
        
    // Load triangle normal.
    float3 normal;
    float3 objectNormal;
    {
        // Retrieve corresponding vertex normals for the triangle vertices.
        float3 vertexNormals[3] = { vertices[0].Normal, vertices[1].Normal, vertices[2].Normal };
        objectNormal = normalize(HitAttribute(vertexNormals, attr));

        float orientation = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE ? 1 : -1;
        objectNormal *= orientation;

        // BLAS Transforms in this sample are uniformly scaled so it's OK to directly apply the BLAS transform.
        float3x3 wit = (float3x3)l_worldIT;
        normal = normalize(mul((float3x3) wit, objectNormal));
    }
    float3 hitPosition = HitWorldPosition();

#define BitMasked(value, bitIdx) value & (1u << bitIdx)
        
    if (BitMasked(l_TexStats, MaterialTextureType::Normal))
    {
        normal = NormalMap(normal, texCoord, vertices, attr);
        normal = normalize(normal);
    }

    if (BitMasked(l_TexStats, MaterialTextureType::Diffuse))
    {
        float4 texSample = l_TexDiffuse.SampleLevel(l_DiffuseSampler, texCoord, 0);
        material.Albedo = texSample.xyz;
        material.Transmissivity = material.Albedo * (1 - texSample.w);
    }
    else
    {
        material.Albedo = l_DiffuseAlbedo.xyz;
        material.Transmissivity = material.Albedo * (1 - l_DiffuseAlbedo.w);
    }
  
    if (BitMasked(l_TexStats, MaterialTextureType::Metallic))
    {
        float texSample = l_TexMetallic.SampleLevel(l_MetallicSampler, texCoord, 0);
        material.Metallic = texSample * material.Albedo;
    }
    else
        material.Metallic = l_Metallic * material.Albedo;
       
    if (BitMasked(l_TexStats, MaterialTextureType::Emissive))
    {
        float3 texSample = l_TexEmissive.SampleLevel(l_EmissiveSampler, texCoord, 0).xyz;
        material.Emission = texSample;
    }
    else
        material.Emission = l_Emissive;
  
    if (BitMasked(l_TexStats, MaterialTextureType::Roughness))
    {
        float texSample = l_TexRoughness.SampleLevel(l_RoughnessSampler, texCoord, 0);
        material.Rroughness = texSample;
    }
    else
        material.Rroughness = l_Roughness;
            
    material.Type = MaterialType::Default;
        
    rayPayload.GBuffer.THit = RayTCurrent();
    rayPayload.GBuffer.HitPosition = hitPosition;
    rayPayload.GBuffer.EncodedNormal = EncodeNormal(normal);
                
    
    material.Specular = lerp(kDielectricSpecular, material.Albedo, material.Metallic);
    material.Albedo = material.Albedo * (1 - kDielectricSpecular) * (1 - material.Metallic);
    material.Opacity = 1;

    // Shade the current hit point, including casting any further rays into the scene 
    // based on current's surface material properties.
    rayPayload.Radiance = Shade(rayPayload, normal, objectNormal, hitPosition, material);
}