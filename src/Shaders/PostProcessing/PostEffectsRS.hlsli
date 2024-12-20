//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#ifndef __POSTEFFECT_RS_HLSLI__
#define __POSTEFFECT_RS_HLSLI__

#define PostEffects_RootSig \
    "RootFlags(0), " \
    "RootConstants(b0, num32BitConstants = 5), " \
    "DescriptorTable(UAV(u0, numDescriptors = 4))," \
    "DescriptorTable(SRV(t0, numDescriptors = 4))," \
    "CBV(b1)," \
    "CBV(b2)," \
    "CBV(b3)," \
    "StaticSampler(s0," \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s1," \
        "addressU = TEXTURE_ADDRESS_BORDER," \
        "addressV = TEXTURE_ADDRESS_BORDER," \
        "addressW = TEXTURE_ADDRESS_BORDER," \
        "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR)," \
    "StaticSampler(s2, " \
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)," \

SamplerState LinearSampler : register(s0);
SamplerState LinearSamplerBorderBlack : register(s1);
SamplerState BilinearSampler : register(s2);

#endif
