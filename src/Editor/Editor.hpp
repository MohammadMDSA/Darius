//
// Game.h
//

#pragma once


#include <Core/TimeManager/StepTimer.hpp>
#include <Renderer/DeviceResources.hpp>
#include <Renderer/FrameResource.hpp>
#include <Math/Camera/Camera.hpp>

#ifndef D_EDITOR
#define D_EDITOR Darius::Editor
#endif // !D_EDITOR


namespace Darius::Editor
{

    // A basic game implementation that creates a D3D12 device and
    // provides a game loop.
    class Editor final : public D_DEVICE_RESOURCE::IDeviceNotify
    {
    public:

        Editor() noexcept(false);
        ~Editor();

        Editor(Editor&&) = default;
        Editor& operator= (Editor&&) = default;

        Editor(Editor const&) = delete;
        Editor& operator= (Editor const&) = delete;

        // Initialization and management
        void Initialize(HWND window, int width, int height);

        // Basic game loop
        void Tick();

        // IDeviceNotify
        void OnDeviceLost() override;
        void OnDeviceRestored() override;

        // Messages
        void OnActivated();
        void OnDeactivated();
        void OnSuspending();
        void OnResuming();
        void OnWindowMoved();
        void OnDisplayChange();
        void OnWindowSizeChanged(int width, int height);

        // Properties
        void GetDefaultSize(int& width, int& height) const noexcept;

    private:

        void Update(D_TIME::StepTimer const& timer);
        void Render();

        void UpdateRotation();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

        ///////////////////////////////////////////
        void InitMesh();
        void BuildDescriptorHeaps();
        void BuildConstantBuffers();
        void BuildRootSignature();
        void BuildShadersAndInputLayout();
        void BuildGeometery();
        void BuildPSO();
        void BuildRenderItems();
        void DisposeUploadBuffers();

        ///////////////////////////////////////////


        // If using the DirectX Tool Kit for DX12, uncomment this line:
        // std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

        std::vector<std::unique_ptr<D_RENDERER_FRAME_RESOUCE::RenderItem>> mRenderItems;
        std::unique_ptr<Mesh>                       mMesh;
        std::unique_ptr<D_MATH_CAMERA::Camera>      mCamera;

        float                                       mWidth;
        float                                       mHeight;
    };
}