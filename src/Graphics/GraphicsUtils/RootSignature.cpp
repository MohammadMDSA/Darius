#include "Graphics/pch.hpp"
#include "RootSignature.hpp"

#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

#include <Core/Hash.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Log.hpp>

#include <map>
#include <thread>
#include <mutex>

using namespace std;
using Microsoft::WRL::ComPtr;

static std::map< size_t, ComPtr<ID3D12RootSignature> > s_RootSignatureHashMap;

namespace
{
    // Root parameter costs in DWORDs as described here: https://docs.microsoft.com/en-us/windows/desktop/direct3d12/root-signature-limits
    static const UINT32 RootDescriptorTableCostGlobal = 1; // Descriptor tables cost 1 DWORD
    static const UINT32 RootDescriptorTableCostLocal = 2; // Local root signature descriptor tables cost 2 DWORDs -- undocumented as of 2018-11-12
    static const UINT32 RootConstantCost = 1; // Each root constant is 1 DWORD
    static const UINT32 RootDescriptorCost = 2; // Root descriptor is 64-bit GPU virtual address, 2 DWORDs
}

namespace Darius::Graphics::Utils
{
    void RootSignature::DestroyAll(void)
    {
        s_RootSignatureHashMap.clear();
    }

    void RootSignature::InitStaticSampler(
        UINT Register,
        const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
        D3D12_SHADER_VISIBILITY Visibility)
    {
        D_ASSERT(mNumInitializedStaticSamplers < mNumSamplers);
        D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = mSamplerArray[mNumInitializedStaticSamplers++];

        StaticSamplerDesc.Filter = NonStaticSamplerDesc.Filter;
        StaticSamplerDesc.AddressU = NonStaticSamplerDesc.AddressU;
        StaticSamplerDesc.AddressV = NonStaticSamplerDesc.AddressV;
        StaticSamplerDesc.AddressW = NonStaticSamplerDesc.AddressW;
        StaticSamplerDesc.MipLODBias = NonStaticSamplerDesc.MipLODBias;
        StaticSamplerDesc.MaxAnisotropy = NonStaticSamplerDesc.MaxAnisotropy;
        StaticSamplerDesc.ComparisonFunc = NonStaticSamplerDesc.ComparisonFunc;
        StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        StaticSamplerDesc.MinLOD = NonStaticSamplerDesc.MinLOD;
        StaticSamplerDesc.MaxLOD = NonStaticSamplerDesc.MaxLOD;
        StaticSamplerDesc.ShaderRegister = Register;
        StaticSamplerDesc.RegisterSpace = 0;
        StaticSamplerDesc.ShaderVisibility = Visibility;

        if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
            StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
            StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
        {
            WARN_ONCE_IF_NOT(
                // Transparent Black
                NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
                NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
                NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
                NonStaticSamplerDesc.BorderColor[3] == 0.0f ||
                // Opaque Black
                NonStaticSamplerDesc.BorderColor[0] == 0.0f &&
                NonStaticSamplerDesc.BorderColor[1] == 0.0f &&
                NonStaticSamplerDesc.BorderColor[2] == 0.0f &&
                NonStaticSamplerDesc.BorderColor[3] == 1.0f ||
                // Opaque White
                NonStaticSamplerDesc.BorderColor[0] == 1.0f &&
                NonStaticSamplerDesc.BorderColor[1] == 1.0f &&
                NonStaticSamplerDesc.BorderColor[2] == 1.0f &&
                NonStaticSamplerDesc.BorderColor[3] == 1.0f,
                "Sampler border color does not match static sampler limitations");

            if (NonStaticSamplerDesc.BorderColor[3] == 1.0f)
            {
                if (NonStaticSamplerDesc.BorderColor[0] == 1.0f)
                    StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
                else
                    StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            }
            else
                StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        }
    }

    void RootSignature::Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS Flags, bool build)
    {
        if (mFinalized)
            return;

        D_ASSERT(mNumInitializedStaticSamplers == mNumSamplers);

        D3D12_ROOT_SIGNATURE_DESC RootDesc;
        RootDesc.NumParameters = mNumParameters;
        RootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)mParamArray.get();
        RootDesc.NumStaticSamplers = mNumSamplers;
        RootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)mSamplerArray.get();
        RootDesc.Flags = Flags;

        mDescriptorTableBitMap = 0;
        mSamplerTableBitMap = 0;

        size_t HashCode = D_CORE::HashState(&RootDesc.Flags);
        HashCode = D_CORE::HashState(RootDesc.pStaticSamplers, mNumSamplers, HashCode);

        for (UINT Param = 0; Param < mNumParameters; ++Param)
        {
            const D3D12_ROOT_PARAMETER& RootParam = RootDesc.pParameters[Param];
            mDescriptorTableSize[Param] = 0;

            if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                D_ASSERT(RootParam.DescriptorTable.pDescriptorRanges != nullptr);

                HashCode = D_CORE::HashState(RootParam.DescriptorTable.pDescriptorRanges,
                    RootParam.DescriptorTable.NumDescriptorRanges, HashCode);

                // We keep track of sampler descriptor tables separately from CBV_SRV_UAV descriptor tables
                if (RootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                    mSamplerTableBitMap |= (1 << Param);
                else
                    mDescriptorTableBitMap |= (1 << Param);

                for (UINT TableRange = 0; TableRange < RootParam.DescriptorTable.NumDescriptorRanges; ++TableRange)
                    mDescriptorTableSize[Param] += RootParam.DescriptorTable.pDescriptorRanges[TableRange].NumDescriptors;
            }
            else
                HashCode = D_CORE::HashState(&RootParam, 1, HashCode);
        }

        // Calculating root parameters size
        {
            mTotalRootSignatureSizeInDWORDs = CalculateTotalRootSignatureSizeInDWORDs(RootDesc);
        }

        ID3D12RootSignature** RSRef = nullptr;
        bool firstCompile = false;
        {
            static mutex s_HashMapMutex;
            lock_guard<mutex> CS(s_HashMapMutex);
            auto iter = s_RootSignatureHashMap.find(HashCode);

            // Reserve space so the next inquiry will find that someone got here first.
            if (iter == s_RootSignatureHashMap.end())
            {
                RSRef = s_RootSignatureHashMap[HashCode].GetAddressOf();
                firstCompile = true;
            }
            else
                RSRef = iter->second.GetAddressOf();
        }

        if (build)
        {
            if (firstCompile)
            {
                ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

                auto hr = D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                    pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf());

                if (pErrorBlob != nullptr)
                {
                    D_LOG_FATAL((char*)pErrorBlob->GetBufferPointer());
                }
                D_HR_CHECK(hr);

                D_HR_CHECK(D_GRAPHICS_DEVICE::GetDevice()->CreateRootSignature(1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
                    IID_PPV_ARGS(&mSignature)));

                mSignature->SetName(name.c_str());

                s_RootSignatureHashMap[HashCode].Attach(mSignature);
                D_ASSERT(*RSRef == mSignature);
            }
            else
            {
                while (*RSRef == nullptr)
                    this_thread::yield();
                mSignature = *RSRef;
            }
            mFinalized = TRUE;
        }
    }

    UINT8 RootSignature::CalculateTotalRootSignatureSizeInDWORDs(D3D12_ROOT_SIGNATURE_DESC RootDesc)
    {
        const UINT32 RootDescriptorTableCost = (RootDesc.Flags & D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE) ? RootDescriptorTableCostLocal : RootDescriptorTableCostGlobal;

        UINT8 size = 0u;

        // Loop through each root parameter and add its size to the total
        for (UINT i = 0; i < RootDesc.NumParameters; i++)
        {
            const D3D12_ROOT_PARAMETER& param = RootDesc.pParameters[i];

            switch (param.ParameterType)
            {
            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                break;
                size += RootConstantCost * param.Constants.Num32BitValues;
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                size += RootDescriptorTableCost;
                break;
            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
                size += RootDescriptorCost;
                break;
            default:
                break;
            }
        }

        return size;
    }

}