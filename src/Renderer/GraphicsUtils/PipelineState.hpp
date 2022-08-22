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

#pragma once

class CommandContext;
class RootSignature;
class VertexShader;
class GeometryShader;
class HullShader;
class DomainShader;
class PixelShader;
class ComputeShader;

#include "RootSignature.hpp"

#include <Utils/Assert.hpp>

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils
{
    class PSO
    {
    public:

        PSO(const wchar_t* Name) : mName(Name), mRootSignature(nullptr), mPSO(nullptr) {}

        static void DestroyAll(void);

        void SetRootSignature(const RootSignature& BindMappings)
        {
            mRootSignature = &BindMappings;
        }

        const RootSignature& GetRootSignature(void) const
        {
            D_ASSERT(mRootSignature != nullptr);
            return *mRootSignature;
        }

        ID3D12PipelineState* GetPipelineStateObject(void) const { return mPSO; }

    protected:

        const wchar_t* mName;

        const RootSignature* mRootSignature;

        ID3D12PipelineState* mPSO;
    };

    class GraphicsPSO : public PSO
    {
        friend class CommandContext;

    public:

        // Start with empty state
        GraphicsPSO(const wchar_t* Name = L"Unnamed Graphics PSO");

        void SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
        void SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc);
        void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);
        void SetSampleMask(UINT SampleMask);
        void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TopologyType);
        void SetDepthTargetFormat(DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
        void SetRenderTargetFormat(DXGI_FORMAT RTVFormat, DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
        void SetRenderTargetFormats(UINT NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat, UINT MsaaCount = 1, UINT MsaaQuality = 0);
        void SetInputLayout(UINT NumElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs);
        void SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBProps);

        // These const_casts shouldn't be necessary, but we need to fix the API to accept "const void* pShaderBytecode"
        void SetVertexShader(const void* Binary, size_t Size) { mPSODesc.VS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetPixelShader(const void* Binary, size_t Size) { mPSODesc.PS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetGeometryShader(const void* Binary, size_t Size) { mPSODesc.GS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetHullShader(const void* Binary, size_t Size) { mPSODesc.HS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetDomainShader(const void* Binary, size_t Size) { mPSODesc.DS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }

        void SetVertexShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.VS = Binary; }
        void SetPixelShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.PS = Binary; }
        void SetGeometryShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.GS = Binary; }
        void SetHullShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.HS = Binary; }
        void SetDomainShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.DS = Binary; }

        // Perform validation and compute a hash value for fast state block comparisons
        void Finalize(std::wstring nameOverride = L"");

    private:

        D3D12_GRAPHICS_PIPELINE_STATE_DESC mPSODesc;
        std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> mInputLayouts;
    };


    class ComputePSO : public PSO
    {
        friend class CommandContext;

    public:
        ComputePSO(const wchar_t* Name = L"Unnamed Compute PSO");

        void SetComputeShader(const void* Binary, size_t Size) { mPSODesc.CS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(Binary), Size); }
        void SetComputeShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.CS = Binary; }

        void Finalize();

    private:

        D3D12_COMPUTE_PIPELINE_STATE_DESC mPSODesc;
    };

}