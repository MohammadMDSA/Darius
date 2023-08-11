
#ifndef __RAYTRACINGHLSLCOMPAT_H__
#define __RAYTRACINGHLSLCOMPAT_H__

#define HitDistanceOnMiss 0
#define NEAR_PLANE 0.001f
#define FAR_PLANE 1000.0f

#ifdef HLSL
#include "Utils/HlslCompat.h"
#else
using namespace DirectX;
#endif


struct SceneConstantBuffer
{
    XMMATRIX gProjectionToWorld;
    XMVECTOR gCameraPosition;
    XMVECTOR gLightAmbientColor;
};

struct SimplePayload
{
    XMFLOAT3 color;
};

struct Ray
{
    XMFLOAT3 Origin;
    XMFLOAT3 Direction;
};

#ifdef HLSL
typedef uint NormalDepthTexFormat;
#else
#define COMPACT_NORMAL_DEPTH_DXGI_FORMAT DXGI_FORMAT_R32_UINT
#endif

// GBuffer data collected during pathtracing 
struct PathTracerGBuffer
{
    float                           THit;
    XMFLOAT3                        HitPosition;            // Position of the hit for which to calculate Ambient coefficient.
    UINT                            DiffuseByte3;           // Diffuse reflectivity of the hit surface.
    // TODO: RTAO pipeline uses 16b encoded normal, therefore same bit enconding could be applied here 
    //  to lower the struct's size and potentially improve Pathtracer's perf without much/any quality loss in RTAO.
    //  Furthermore, _encodedNormal below could use lower bit range too.
    XMFLOAT2                        EncodedNormal;          // Normal of the hit surface. 

    // Members for Motion Vector calculation.
    //XMFLOAT3                        _virtualHitPosition;    // virtual hitPosition in the previous frame.
    // For non-reflected points this is a true world position of a hit.
    // For reflected points, this is a world position of a hit reflected across the reflected surface 
    //   ultimately giving the same screen space coords when projected and the depth corresponding to the ray depth.
    //XMFLOAT2                        _encodedNormal;         // surface normal in the previous frame
};


struct PathTracerRayPayload
{
    UINT                            RayRecursionDepth;
    XMFLOAT3                        Radiance;              // TODO encode
    PathTracerGBuffer               GBuffer;
};

struct ShadowRayPayload
{
    float                           THit;         // Hit time <0,..> on Hit. -1 on miss.
};

struct RTVertexPositionNormalTangentTexture
{
    XMFLOAT3						Position;
    XMFLOAT3						Normal;
    XMFLOAT4						Tangent;
    XMFLOAT2						Tex;
};

struct VertexPositionNormalTangentTextureSkinned
{
    XMFLOAT3						Position;
    XMFLOAT3						Normal;
    XMFLOAT4						Tangent;
    XMFLOAT2						Tex;
    XMUINT4						    BlendIndices;
    XMFLOAT4						BlendWeights;
};

#endif
