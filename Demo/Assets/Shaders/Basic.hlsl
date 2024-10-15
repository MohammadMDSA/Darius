#include "MaterialShaders/MaterialShadersCommon.hlsli"


//struct MaterialConstantsType
//{
    float   f1;
    float   f2;
    int     i1;
    bool    b1;
    float2  v2;
    float3  v3;
    float4  v4;
//};

//ParamDeclaration(MaterialConstantsType)

float __main__() : SV_Target
{
    return f1;
}