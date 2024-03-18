
#ifndef __JOINT_HLSLI__
#define __JOINT_HLSLI__

struct Joint
{
    float4x4 PosMatrix;
    float3x3 NrmMatrix; // Inverse-transpose of PosMatrix
};

#endif
