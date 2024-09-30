#pragma once

#include "RootSignature.hpp"

#include <Core/Signal.hpp>
#include <Utils/Assert.hpp>

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics
{
    class CommandContext;
}

namespace Darius::Graphics::Utils
{
    namespace Shaders
    {
        class ComputeShader;
        class VertexShader;
        class PixelShader;
        class GeometryShader;
        class HullShader;
        class DomainShader;
    }

    class RootSignature;

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

        wchar_t const* GetName() const
        {
            return mName;
        }

    protected:

        template <typename SHADER>
        struct PsoShaderData
        {
            PsoShaderData(std::shared_ptr<SHADER> shader) : Shader(shader) {}
            INLINE ~PsoShaderData() { OnShaderCompiledConnection.disconnect(); }

        public:
            std::shared_ptr<SHADER>         Shader;
            D_CORE::SignalConnection        OnShaderCompiledConnection;
        };

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
        void SetVertexShader(std::shared_ptr<Shaders::VertexShader> shader);
        void SetPixelShader(std::shared_ptr<Shaders::PixelShader> shader);
        void SetGeometryShader(std::shared_ptr<Shaders::GeometryShader> shader);
        void SetHullShader(std::shared_ptr<Shaders::HullShader> shader);
        void SetDomainShader(std::shared_ptr<Shaders::DomainShader> shader);

        void SetVertexShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.VS = Binary; }
        void SetPixelShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.PS = Binary; }
        void SetGeometryShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.GS = Binary; }
        void SetHullShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.HS = Binary; }
        void SetDomainShader(const D3D12_SHADER_BYTECODE& Binary) { mPSODesc.DS = Binary; }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC const& GetDesc() const { return mPSODesc; }

        // Perform validation and compute a hash value for fast state block comparisons
        void Finalize(std::wstring nameOverride = L"");

    private:

        void OnVertexShaderRecompiled() {}
        void OnPixelShaderRecompiled() {}
        void OnGeometryShaderRecompiled() {}
        void OnHullShaderRecompiled() {}
        void OnDomainShaderRecompiled() {}

        D3D12_GRAPHICS_PIPELINE_STATE_DESC mPSODesc;
        std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> mInputLayouts;
        PsoShaderData<Shaders::VertexShader>        mVertexShader;
        PsoShaderData<Shaders::PixelShader>         mPixelShader;
        PsoShaderData<Shaders::GeometryShader>      mGeometryShader;
        PsoShaderData<Shaders::HullShader>          mHullShader;
        PsoShaderData<Shaders::DomainShader>        mDomainShader;
    };


    class ComputePSO : public PSO
    {
        friend class CommandContext;

    public:
        ComputePSO(const wchar_t* Name = L"Unnamed Compute PSO");

        void SetComputeShader(std::shared_ptr<Shaders::ComputeShader> computeShader);

        INLINE std::shared_ptr<Shaders::ComputeShader> GetComputeShader() const { return mComputeShader.Shader; }

        void OnComputeShaderRecompiled() {}

        void Finalize();

    private:

        D3D12_COMPUTE_PIPELINE_STATE_DESC       mPSODesc;
        PsoShaderData<Shaders::ComputeShader>   mComputeShader;
    };

}