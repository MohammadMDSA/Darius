#include "../../../Geometry/GenericVertexType.hlsli"

#ifdef ENABLE_SKINNING
#include "../../../Geometry/Joint.hlsli"
#endif

struct DrawConstants
{
    float4x4 WorldMatrix;
    float3x3 WorldITMatrix;
};

ConstantBuffer<DrawConstants> DrawCB;

#include "ForwardPassConstants.hlsli"

#ifdef ENABLE_SKINNING
StructuredBuffer<Joint> Joints : register(t20);
#endif


GenericVSOutputVertexLayout Main(GenericVSInputVertexLayout vin)
{
    GenericVSOutputVertexLayout vout;
    
    // Transform to world space
    float4 position = float4(vin.LocalPosition, 1.f);
    float3 normal = vin.Normal;
    
#ifndef NO_TANGENT_FRAME
    float4 tangent = vin.Tangent;
#endif

#ifdef ENABLE_SKINNING
    float4 weights = vin.JointWeights / dot(vin.JointWeights, 1);
    
    float4x4 skinPosMat =
        Joints[vin.JointIndices.x].PosMatrix * weights.x +
        Joints[vin.JointIndices.y].PosMatrix * weights.y +
        Joints[vin.JointIndices.z].PosMatrix * weights.z +
        Joints[vin.JointIndices.w].PosMatrix * weights.w;

    position = mul(skinPosMat, position);

    float3x3 skinNrmMat =
        Joints[vin.JointIndices.x].NrmMatrix * weights.x +
        Joints[vin.JointIndices.y].NrmMatrix * weights.y +
        Joints[vin.JointIndices.z].NrmMatrix * weights.z +
        Joints[vin.JointIndices.w].NrmMatrix * weights.w;

    normal = mul(skinNrmMat, normal).xyz;
    
#ifndef NO_TANGENT_FRAME
    tangent.xyz = mul(skinNrmMat, tangent.xyz).xyz;
#endif
#endif
    
    vout.LocalNormal = normal;
    vout.LocalPosition = position.xyz;
    
#ifndef WORLD_DISPLACEMENT
    vout.WorldPosition = mul(DrawCB.WorldMatrix, position).xyz;
    vout.WorldNormal = mul(DrawCB.WorldITMatrix, normal);
    
    // Transform to homogeneous clip space
    vout.ScreenNDC = mul(ForwardPassCB.WorldToClipMatrix, float4(vout.WorldPosition, 1.f));
#endif
    
#ifndef NO_TANGENT_FRAME
    vout.Tangent = float4(mul(DrawCB.WorldITMatrix, tangent.xyz), tangent.w);
#endif
    
#ifdef HAS_UV0
    vout.UV0 = vin.UV0;
#ifdef HAS_UV1
    vout.UV1 = vin.UV1;
#ifdef HAS_UV2
    vout.UV2 = vin.UV2;
#ifdef HAS_UV3
    vout.UV3 = vin.UV3;
#endif
#endif
#endif
#endif
    
    return vout;
}
