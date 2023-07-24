#pragma once

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils
{
	class StateObject
	{
	public:
		StateObject(D3D12_STATE_OBJECT_TYPE type) :
			mType(type),
			mStateObject(nullptr),
			mFinalized(false)
		{}

		virtual D3D12_STATE_OBJECT_DESC			GetDesc() const = 0;

		void									Finalize(std::wstring const& name);

		static void								DestroyAll();

	private:
		D3D12_STATE_OBJECT_TYPE					mType;
		ID3D12StateObject*						mStateObject;
		bool									mFinalized;
	};

	class RayTracingStateObject : public StateObject
	{
	public:
		RayTracingStateObject(UINT pipelineMaxTraceRecursionDepth, UINT shaderMaxPayloadSizeInBytes, UINT shaderMaxAttributeSizeInBytes = sizeof(DirectX::XMFLOAT2)) :
			StateObject(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE),
			mMaxAttributeSizeInBytes(shaderMaxAttributeSizeInBytes),
			mMaxPayloadSizeInBytes(shaderMaxPayloadSizeInBytes),
			mMaxTraceRecursionDepth(pipelineMaxTraceRecursionDepth)
		{ }

	private:

		// Shader Config
		UINT									mMaxAttributeSizeInBytes;
		UINT									mMaxPayloadSizeInBytes;

		// Pipeline Config
		UINT									mMaxTraceRecursionDepth;

		ID3D12RootSignature*					mGlobalRootSignature;

		UINT									mMaxLocalRootSignatureSize;
	};
}
