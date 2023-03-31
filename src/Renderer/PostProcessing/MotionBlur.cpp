#include "Renderer/pch.hpp"
#include "MotionBlur.hpp"

#include "Renderer/AntiAliasing/TemporalEffect.hpp"
#include "Renderer/GraphicsUtils/PipelineState.hpp"
#include "Renderer/GraphicsUtils/Profiling/Profiling.hpp"

#include <Utils/Assert.hpp>

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;

namespace Darius::Graphics::PostProcessing::MotionBlur
{
	bool								_initialize;

	// PSOs
	ComputePSO							CameraVelocityCS[2] = { { L"Motion Blur: Camera Velocity CS" },{ L"Motion Blur: Camera Velocity Linear Z CS" } };

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialize);
		_initialize = true;

#define CreatePSO(ObjName, ShaderName) \
    { \
        ObjName.SetRootSignature(D_GRAPHICS::CommonRS); \
        auto& shaderData = D_GRAPHICS::Shaders[#ShaderName]; \
        ObjName.SetComputeShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize()); \
        ObjName.Finalize(); \
    }
		CreatePSO(CameraVelocityCS[0], CameraVelocityCS);
		CreatePSO(CameraVelocityCS[1], CameraVelocityCS);

#undef CreatePSO
	}

	void Shutdown()
	{
		D_ASSERT(_initialize);
	}


    void MotionBlur::GenerateCameraVelocityBuffer(CommandContext& context, MotionBlurBuffers& buffers, D_MATH_CAMERA::Camera const& camera, bool useLinearZ)
    {
        GenerateCameraVelocityBuffer(context, buffers, camera.GetReprojectionMatrix(), camera.GetNearClip(), camera.GetFarClip(), useLinearZ);
    }

    void MotionBlur::GenerateCameraVelocityBuffer(CommandContext& context, MotionBlurBuffers& buffers, D_MATH::Matrix4 const& reprojectionMatrix, float nearClip, float farClip, bool useLinearZ)
    {
        D_PROFILING::ScopedTimer _prof(L"Generate Camera Velocity", context);

        ComputeContext& Context = context.GetComputeContext();

        Context.SetRootSignature(D_GRAPHICS::CommonRS);

        uint32_t Width = buffers.SceneColorBuffer.GetWidth();
        uint32_t Height = buffers.SceneColorBuffer.GetHeight();

        float RcpHalfDimX = 2.0f / Width;
        float RcpHalfDimY = 2.0f / Height;
        float RcpZMagic = nearClip / (farClip - nearClip);

        Matrix4 preMult = Matrix4(
            Vector4(RcpHalfDimX, 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, -RcpHalfDimY, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, useLinearZ ? RcpZMagic : 1.0f, 0.0f),
            Vector4(-1.0f, 1.0f, useLinearZ ? -RcpZMagic : 0.0f, 1.0f)
        );

        Matrix4 postMult = Matrix4(
            Vector4(1.0f / RcpHalfDimX, 0.0f, 0.0f, 0.0f),
            Vector4(0.0f, -1.0f / RcpHalfDimY, 0.0f, 0.0f),
            Vector4(0.0f, 0.0f, 1.0f, 0.0f),
            Vector4(1.0f / RcpHalfDimX, 1.0f / RcpHalfDimY, 0.0f, 1.0f));


        DirectX::XMMATRIX CurToPrevXForm = postMult * reprojectionMatrix * preMult;

        Context.SetDynamicConstantBufferView(3, sizeof(CurToPrevXForm), &CurToPrevXForm);
        Context.TransitionResource(buffers.VelocityBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        if (useLinearZ)
            Context.TransitionResource(buffers.LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        else
            Context.TransitionResource(buffers.SceneDepthBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        Context.SetPipelineState(CameraVelocityCS[useLinearZ ? 1 : 0]);
        Context.SetDynamicDescriptor(1, 0, useLinearZ ? buffers.LinearDepth.GetSRV() : buffers.SceneDepthBuffer.GetDepthSRV());
        Context.SetDynamicDescriptor(2, 0, buffers.VelocityBuffer.GetUAV());
        Context.Dispatch2D(Width, Height);
    }

}
