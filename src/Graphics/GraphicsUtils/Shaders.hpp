#pragma once

#include "RootSignature.hpp"

#define D_GRAPHICS_SHADERS Darius::Graphics::Utils::Shaders

namespace Darius::Graphics::Utils::Shaders
{

	namespace Type
	{
		enum EShaderType : UINT16
		{
			Vertex				= 0x1,

			//Tesselation
			Domain				= 0x2,
			Hull				= 0x4,
			TesselationShader	= Domain | Hull,

			Geometry			= 0x8,

			Pixel				= 0x10,

			// Ray Tracing
			RayGeneration		= 0x20,
			Miss				= 0x40,
			ClosestHit			= 0x80,
			AnyHit				= 0x100,
			Intersection		= 0x200,
			RayTracingShader	= RayGeneration | Miss | ClosestHit | AnyHit | Intersection

		};
	}

	class Shader
	{
	public:
		INLINE std::wstring const& GetName() const { return mName; }
		INLINE Type::EShaderType GetType() const { return mType; }

	protected:
		Shader(Type::EShaderType type, std::wstring const& name) :
			mType(type), mName(name)
		{}

	private:
		const Type::EShaderType	mType;
		const std::wstring		mName;
	};

	class VertexShader : public Shader
	{
	public:

		VertexShader(std::wstring const& name) : Shader(Type::Vertex, name) {}
	};

	class DomainShader : public Shader
	{
	public:

		DomainShader(std::wstring const& name) : Shader(Type::Domain, name) {}
	};

	class HullShader : public Shader
	{
	public:

		HullShader(std::wstring const& name) : Shader(Type::Hull, name) {}
	};

	class GeometryShader : public Shader
	{
	public:

		GeometryShader(std::wstring const& name) : Shader(Type::Geometry, name) {}
	};

	class PixelShader : public Shader
	{
	public:

		PixelShader(std::wstring const& name) : Shader(Type::Pixel, name) {}
	};

	class RayTracingShader : public Shader
	{
	public:
		INLINE D3D12_ROOT_SIGNATURE_DESC	GetLocalRootSignatureDesc() const { return mLocalRootDesc; }
		INLINE UINT32						GetRootParametersSizeInBytes() const { return mLocalRootParametersSizeInDWORDs * 4; }
		INLINE UINT8						GetRootParametersSizeInDWORDs() const { return mLocalRootParametersSizeInDWORDs; }

	protected:
		RayTracingShader(D3D12_ROOT_SIGNATURE_DESC localRT, Type::EShaderType type, std::wstring const& name) :
			Shader(type, name),
			mLocalRootDesc(localRT)
		{
			D_ASSERT(type | Type::RayTracingShader);
			mLocalRootParametersSizeInDWORDs = RootSignature::CalculateTotalRootSignatureSizeInDWORDs(localRT);
		}

	private:
		D3D12_ROOT_SIGNATURE_DESC		mLocalRootDesc;
		UINT8							mLocalRootParametersSizeInDWORDs;
	};

	class RayGenerationShader : public RayTracingShader
	{
	public:

		RayGenerationShader(std::wstring const& name, D3D12_ROOT_SIGNATURE_DESC localRT) : RayTracingShader(localRT, Type::RayGeneration, name) {}
	};

	class MissShader : public RayTracingShader
	{
	public:

		MissShader(std::wstring const& name, D3D12_ROOT_SIGNATURE_DESC localRT) : RayTracingShader(localRT, Type::Miss, name) {}
	};

	class ClosestHitShader : public RayTracingShader
	{
	public:

		ClosestHitShader(std::wstring const& name, D3D12_ROOT_SIGNATURE_DESC localRT) : RayTracingShader(localRT, Type::ClosestHit, name) {}
	};

	class AnyHitShader : public RayTracingShader
	{
	public:

		AnyHitShader(std::wstring const& name, D3D12_ROOT_SIGNATURE_DESC localRT) : RayTracingShader(localRT, Type::AnyHit, name) {}
	};

	class IntersectionShader : public RayTracingShader
	{
	public:

		IntersectionShader(std::wstring const& name, D3D12_ROOT_SIGNATURE_DESC localRT) : RayTracingShader(localRT, Type::Intersection, name) {}
	};

}