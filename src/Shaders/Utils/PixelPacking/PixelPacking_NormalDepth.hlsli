#ifndef __PIXEL_PACKING_NORMALDEPTH_HLSLI__
#define __PIXEL_PACKING_NORMALDEPTH_HLSLI__

// Pack [0.0, 1.0] float to 8 bit uint. 
uint Pack_R8_FLOAT(float r)
{
    return clamp(round(r * 255), 0, 255);
}

float Unpack_R8_FLOAT(uint r)
{
    return (r & 0xFF) / 255.0;
}

// Pack unsigned floating point, where 
// - rgb.rg are in [0, 1] range stored as two 8 bit uints.
// - rgb.b in [0, FLT_16_BIT_MAX] range stored as a 16bit float.
uint Pack_R8G8B16_FLOAT(float3 rgb)
{
    uint r = Pack_R8_FLOAT(rgb.r);
    uint g = Pack_R8_FLOAT(rgb.g) << 8;
    uint b = f32tof16(rgb.b) << 16;
    return r | g | b;
}

float3 Unpack_R8G8B16_FLOAT(uint rgb)
{
    float r = Unpack_R8_FLOAT(rgb);
    float g = Unpack_R8_FLOAT(rgb >> 8);
    float b = f16tof32(rgb >> 16);
    return float3(r, g, b);
}

/***************************************************************/
// Normal encoding
// Ref: https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
float2 OctWrap(float2 v)
{
    return (1.0 - abs(v.yx)) * select(v.xy >= 0.0, 1.0, -1.0);
}

// Converts a 3D unit vector to a 2D vector with <0,1> range. 
float2 EncodeNormal(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}

float3 DecodeNormal(float2 f)
{
    f = f * 2.0 - 1.0;

    // https://twitter.com/Stubbesaurus/status/937994790553227264
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    n.xy += select(n.xy >= 0.0, -t, t);
    return normalize(n);
}
/***************************************************************/

// Encode normal and depth with 16 bits allocated for each.
uint EncodeNormalDepth_N16D16(in float3 normal, in float depth)
{
    float3 encodedNormalDepth = float3(EncodeNormal(normal), depth);
    return Pack_R8G8B16_FLOAT(encodedNormalDepth);
}


// Decoded 16 bit normal and 16bit depth.
void DecodeNormalDepth_N16D16(in uint packedEncodedNormalAndDepth, out float3 normal, out float depth)
{
    float3 encodedNormalDepth = Unpack_R8G8B16_FLOAT(packedEncodedNormalAndDepth);
    normal = DecodeNormal(encodedNormalDepth.xy);
    depth = encodedNormalDepth.z;
}

uint EncodeNormalDepth(in float3 normal, in float depth)
{
    return EncodeNormalDepth_N16D16(normal, depth);
}

void DecodeNormalDepth(in uint encodedNormalDepth, out float3 normal, out float depth)
{
    DecodeNormalDepth_N16D16(encodedNormalDepth, normal, depth);
}

void DecodeNormal(in uint encodedNormalDepth, out float3 normal)
{
    float depthDummy;
    DecodeNormalDepth_N16D16(encodedNormalDepth, normal, depthDummy);
}

void UnpackEncodedNormalDepth(in uint packedEncodedNormalDepth, out float2 encodedNormal, out float depth)
{
    float3 encodedNormalDepth = Unpack_R8G8B16_FLOAT(packedEncodedNormalDepth);
    encodedNormal = encodedNormalDepth.xy;
    depth = encodedNormalDepth.z;
}

uint NormalizedFloat3ToByte3(float3 v)
{
    return
        (uint(v.x * 255) << 16) +
        (uint(v.y * 255) << 8) +
        uint(v.z * 255);
}

float3 Byte3ToNormalizedFloat3(uint v)
{
    return float3(
        (v >> 16) & 0xff,
        (v >> 8) & 0xff,
        v & 0xff) / 255;
}

#endif
