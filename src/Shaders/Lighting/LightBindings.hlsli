#ifndef __LIGHT_BINDINGS_HLSLI__
#define __LIGHT_BINDINGS_HLSLI__

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
    float3          Color;
    float3          Direction;
    float3          Position;
    float           Intencity;
    float           Radius;
    float2          SpotAngles; // x = 1.0f / (cos(coneInner) - cos(coneOuter)), y = cos(coneOuter)
    bool            CastsShadow;
    int3            padding;
};

ByteAddressBuffer               g_LightMask : register(t10, space0);
StructuredBuffer<Light>         g_LightData : register(t11, space0);

#endif
