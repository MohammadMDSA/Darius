#ifndef __FORWARD_PASS_CONSTANTS_HLSLI__
#define __FORWARD_PASS_CONSTANTS_HLSLI__

struct ForwardPassConstants
{
    float4x4                    WorldToViewMatrix; // View Mat
    float4x4                    ViewToWorldMatrix; // Inv View Mat
    float4x4                    ViewToClipMatrix; // Proj Mat
    float4x4                    ClipToViewMatrix; // Inv Proj Mat
    float4x4                    WorldToClipMatrix; // View Proj Mat
    float4x4                    ClipToWorldMatrix; // Inv View Proj Mat;
    
    float4                      FrustumPlanes[6];
    float3                      CameraWorldPosition;
    ////

    float2                      RenderTargetSize;
    float2                      InvRenderTargetSize;
    ////
    
    float                       NearClip;
    float                       FarClip;

    float                       Time;
    float                       DeltaTime;
    ////
    
    float4                      AmbientLight;
    
    float                       IBLRange;
    float                       IBLBias;  
};

ConstantBuffer<ForwardPassConstants>    ForwardPassCB;

#endif