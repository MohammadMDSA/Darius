//
// DeviceResources.h - A wrapper for the Direct3D 12 device and swapchain
//

#pragma once
#include "./pch.hpp"

#include "FrameResource.hpp"

#include <Core/Signal.hpp>
#include <Utils/Assert.hpp>

#define D_DEVICE_RESOURCE Darius::Renderer::DeviceResource

using namespace Darius::Renderer;
using namespace Darius::Renderer::ConstantFrameResource;

namespace Darius::Renderer::DeviceResource
{
    // Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
    interface IDeviceNotify
    {
        virtual void OnDeviceLost() = 0;
        virtual void OnDeviceRestored() = 0;

    protected:
        ~IDeviceNotify() = default;
    };

    // Controls all the DirectX device resources.
    class DeviceResources
    {
    public:
        static constexpr unsigned int c_AllowTearing = 0x1;
        static constexpr unsigned int c_EnableHDR = 0x2;

        DeviceResources(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT,
            UINT backBufferCount = 2,
            D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_11_0,
            unsigned int flags = 0) noexcept(false);
        ~DeviceResources();

        DeviceResources(DeviceResources&&) = default;
        DeviceResources& operator= (DeviceResources&&) = default;

        DeviceResources(DeviceResources const&) = delete;
        DeviceResources& operator= (DeviceResources const&) = delete;

        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void SetWindow(HWND window, int width, int height) noexcept;
        bool WindowSizeChanged(int width, int height);
        void HandleDeviceLost();
        void RegisterDeviceNotify(IDeviceNotify* deviceNotify) noexcept
        {
            m_deviceLostSignal.connect(boost::bind(&IDeviceNotify::OnDeviceLost, deviceNotify));
            m_deviceRestoredSignal.connect(boost::bind(&IDeviceNotify::OnDeviceRestored, deviceNotify));
        }
        void Prepare(ID3D12PipelineState* pso = nullptr, D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void Present(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
        void WaitForGpu() noexcept;
        void UpdateColorSpace();

        // Device Accessors.
        RECT GetOutputSize() const noexcept { return m_outputSize; }

        // Direct3D Accessors.
        auto                        GetD3DDevice() const noexcept { return m_d3dDevice.Get(); }
        auto                        GetSwapChain() const noexcept { return m_swapChain.Get(); }
        auto                        GetDXGIFactory() const noexcept { return m_dxgiFactory.Get(); }
        HWND                        GetWindow() const noexcept { return m_window; }
        D3D_FEATURE_LEVEL           GetDeviceFeatureLevel() const noexcept { return m_d3dFeatureLevel; }
        ID3D12Resource* GetRenderTarget() const noexcept { return m_swapChainBuffer[m_backBufferIndex].Get(); }
        ID3D12Resource* GetDepthStencil() const noexcept { return m_depthStencil.Get(); }
        ID3D12CommandQueue* GetCommandQueue() const noexcept { return m_commandQueue.Get(); }
        ID3D12CommandAllocator* GetCommandAllocator() const noexcept { return m_frameResources[m_currentResourceIndex]->CmdListAlloc.Get(); }
        ID3D12CommandAllocator* GetDirectCommandAllocator() const noexcept { return m_directCommandAlloc.Get(); }
        auto                        GetCommandList() const noexcept        { return m_commandList.Get(); }
        DXGI_FORMAT                 GetBackBufferFormat() const noexcept   { return m_backBufferFormat; }
        DXGI_FORMAT                 GetDepthBufferFormat() const noexcept  { return m_depthBufferFormat; }
        D3D12_VIEWPORT              GetScreenViewport() const noexcept     { return m_screenViewport; }
        D3D12_RECT                  GetScissorRect() const noexcept        { return m_scissorRect; }
        UINT                        GetCurrentFrameResourceIndex() const noexcept  { return m_currentResourceIndex; }
        UINT                        GetBackBufferCount() const noexcept    { return m_backBufferCount; }
        UINT                        GetRtvDescriptorSize() const noexcept { return m_rtvDescriptorSize; }
        UINT                        GetDsvDescriptorSize() const noexcept { return m_dsvDescriptorSize; }
        UINT                        GetCbvSrvUavDescriptorSize() const noexcept { return m_cbvSrvUavDescriptorSize; }
        DXGI_COLOR_SPACE_TYPE       GetColorSpace() const noexcept         { return m_colorSpace; }
        unsigned int                GetDeviceOptions() const noexcept      { return m_options; }
        FrameResource*              GetFrameResource() const noexcept      { return m_frameResources[m_currentResourceIndex].get(); }
        FrameResource* GetFrameResourceWithIndex(int i) const noexcept { D_ASSERT(i < gNumFrameResources); return m_frameResources[i].get(); }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const noexcept
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(
                m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                static_cast<INT>(m_backBufferIndex), m_rtvDescriptorSize);
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const noexcept
        {
            return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

        void SyncFrameStartGPU();
    private:

        void MoveToNextFrame(bool parallelGPU = false);
        void GetAdapter(IDXGIAdapter1** ppAdapter);

        UINT                                                m_backBufferIndex;
        UINT                                                m_currentResourceIndex;
        std::array<std::unique_ptr<FrameResource>, D_RENDERER_FRAME_RESOUCE::gNumFrameResources> m_frameResources;


        // Direct3D objects.
        Microsoft::WRL::ComPtr<ID3D12Device>                m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   m_commandList;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>          m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>      m_directCommandAlloc;

        // Swap chain objects.
        Microsoft::WRL::ComPtr<IDXGIFactory4>               m_dxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain3>             m_swapChain;
        // TODO: Change hardcoded array size and make in based on m_backBufferCount
        Microsoft::WRL::ComPtr<ID3D12Resource>              m_swapChainBuffer[2];
        Microsoft::WRL::ComPtr<ID3D12Resource>              m_depthStencil;

        // Presentation fence objects.
        Microsoft::WRL::ComPtr<ID3D12Fence>                 m_fence;
        UINT                                                m_currFenceValue;
        Microsoft::WRL::Wrappers::Event                     m_fenceEvent;

        // Direct3D rendering objects.
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_rtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_dsvDescriptorHeap;

        UINT                                                m_rtvDescriptorSize;
        UINT                                                m_dsvDescriptorSize;
        UINT                                                m_cbvSrvUavDescriptorSize;
        D3D12_VIEWPORT                                      m_screenViewport;
        D3D12_RECT                                          m_scissorRect;

        // Direct3D properties.
        DXGI_FORMAT                                         m_backBufferFormat;
        DXGI_FORMAT                                         m_depthBufferFormat;
        UINT                                                m_backBufferCount;
        D3D_FEATURE_LEVEL                                   m_d3dMinFeatureLevel;

        // Cached device properties.
        HWND                                                m_window;
        D3D_FEATURE_LEVEL                                   m_d3dFeatureLevel;
        DWORD                                               m_dxgiFactoryFlags;
        RECT                                                m_outputSize;

        // HDR Support
        DXGI_COLOR_SPACE_TYPE                               m_colorSpace;

        // DeviceResources options (see flags above)
        unsigned int                                        m_options;

        // The IDeviceNotify can be held directly as it owns the DeviceResources.
        Darius::Engine::Core::Signal<void()>                m_deviceLostSignal;
        Darius::Engine::Core::Signal<void()>                m_deviceRestoredSignal;
    };
}
