#ifndef __VERTEX_TYPE_HLSLI__
#define __VERTEX_TYPE_HLSLI__

#ifdef HLSL
#include "../HlslCompat.h"
#endif

struct RTVertexPositionNormalTangentTexture
{
    XMFLOAT3					Position;
    XMFLOAT3					Normal;
    XMFLOAT4					Tangent;
    XMFLOAT2					Tex;
};

struct VertexPositionNormalTangentTextureSkinned
{
    XMFLOAT3					Position;
    XMFLOAT3					Normal;
    XMFLOAT4					Tangent;
    XMFLOAT2					Tex;
    XMUINT4						BlendIndices;
    XMFLOAT4  					BlendWeights;
};


#endif