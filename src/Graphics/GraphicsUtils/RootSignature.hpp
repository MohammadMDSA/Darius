#pragma once

#include <Utils/Assert.hpp>

class DescriptorCache;

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils::Memory
{
    class DynamicDescriptorHeap;
}

namespace Darius::Graphics::Utils
{
    class RootParameter
    {
        friend class RootSignature;
    public:

        RootParameter()
        {
            m_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
        }

        ~RootParameter()
        {
            Clear();
        }

        void Clear()
        {
            if (m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
                delete[] m_RootParam.DescriptorTable.pDescriptorRanges;

            m_RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
        }

        void InitAsConstants(UINT Register, UINT NumDwords, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
        {
            m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Constants.Num32BitValues = NumDwords;
            m_RootParam.Constants.ShaderRegister = Register;
            m_RootParam.Constants.RegisterSpace = Space;
        }

        void InitAsConstantBuffer(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
        {
            m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Descriptor.ShaderRegister = Register;
            m_RootParam.Descriptor.RegisterSpace = Space;
        }

        void InitAsBufferSRV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
        {
            m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Descriptor.ShaderRegister = Register;
            m_RootParam.Descriptor.RegisterSpace = Space;
        }

        void InitAsBufferUAV(UINT Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
        {
            m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Descriptor.ShaderRegister = Register;
            m_RootParam.Descriptor.RegisterSpace = Space;
        }

        void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL, UINT Space = 0)
        {
            InitAsDescriptorTable(1, Visibility);
            SetTableRange(0, Type, Register, Count, Space);
        }

        void InitAsDescriptorTable(UINT RangeCount, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
        {
            m_RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
            m_RootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[RangeCount];
        }

        void SetTableRange(UINT RangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE Type, UINT Register, UINT Count, UINT Space = 0)
        {
            D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_RootParam.DescriptorTable.pDescriptorRanges + RangeIndex);
            range->RangeType = Type;
            range->NumDescriptors = Count;
            range->BaseShaderRegister = Register;
            range->RegisterSpace = Space;
            range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        }

        const D3D12_ROOT_PARAMETER& operator() (void) const { return m_RootParam; }


    protected:

        D3D12_ROOT_PARAMETER m_RootParam;
    };

    class RootSignature
    {
        friend class Darius::Graphics::Utils::Memory::DynamicDescriptorHeap;

    public:

        RootSignature(UINT NumRootParams = 0, UINT NumStaticSamplers = 0) : mFinalized(FALSE), mNumParameters(NumRootParams), mTotalRootSignatureSizeInDWORDs(0u)
        {
            Reset(NumRootParams, NumStaticSamplers);
        }

        ~RootSignature()
        {
        }

        static void DestroyAll(void);

        static UINT8 CalculateTotalRootSignatureSizeInDWORDs(D3D12_ROOT_SIGNATURE_DESC rtDesc);

        void Reset(UINT NumRootParams, UINT NumStaticSamplers = 0)
        {
            if (NumRootParams > 0)
                mParamArray.reset(new RootParameter[NumRootParams]);
            else
                mParamArray = nullptr;
            mNumParameters = NumRootParams;

            if (NumStaticSamplers > 0)
                mSamplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[NumStaticSamplers]);
            else
                mSamplerArray = nullptr;
            mNumSamplers = NumStaticSamplers;
            mNumInitializedStaticSamplers = 0;
        }

        RootParameter& operator[] (size_t EntryIndex)
        {
            D_ASSERT(EntryIndex < mNumParameters);
            return mParamArray.get()[EntryIndex];
        }

        const RootParameter& operator[] (size_t EntryIndex) const
        {
            D_ASSERT(EntryIndex < mNumParameters);
            return mParamArray.get()[EntryIndex];
        }

        void InitStaticSampler(UINT Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
            D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);

        void Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE, bool build = true);

        ID3D12RootSignature* GetSignature() const { return mSignature; }

    protected:

        BOOL                            mFinalized;
        UINT                            mNumParameters;
        UINT                            mNumSamplers;
        UINT                            mNumInitializedStaticSamplers;
        uint32_t                        mDescriptorTableBitMap; // One bit is set for root parameters that are non-sampler descriptor tables
        uint32_t                        mSamplerTableBitMap; // One bit is set for root parameters that are sampler descriptor tables
        uint32_t                        mDescriptorTableSize[16]; // Non-sampler descriptor tables need to know their descriptor count
        std::unique_ptr<RootParameter[]> mParamArray;
        std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> mSamplerArray;
        ID3D12RootSignature*            mSignature;
        UINT8                           mTotalRootSignatureSizeInDWORDs;
    };
}