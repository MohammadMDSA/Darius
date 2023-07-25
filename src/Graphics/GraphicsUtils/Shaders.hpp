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
		INLINE std::shared_ptr<RootSignature> GetLocalRootSignature() const { return mLocalRootDesc; }
		INLINE UINT32						GetRootParametersSizeInBytes() const { return mLocalRootParametersSizeInDWORDs * 4; }
		INLINE UINT8						GetRootParametersSizeInDWORDs() const { return mLocalRootParametersSizeInDWORDs; }
		INLINE std::wstring const&			GetLibraryName() const { return mLibraryName; }

	protected:
		RayTracingShader(std::shared_ptr<RootSignature> localRT, Type::EShaderType type, std::wstring const& name, std::wstring libraryName) :
			Shader(type, name),
			mLocalRootDesc(localRT),
			mLibraryName(libraryName)
		{
			D_ASSERT(type | Type::RayTracingShader);
			D_ASSERT(localRT->IsFinalized());
			mLocalRootParametersSizeInDWORDs = mLocalRootDesc->GetRootParametersSizeInDWORDs();
		}

	private:
		std::shared_ptr<RootSignature>	mLocalRootDesc;
		UINT8							mLocalRootParametersSizeInDWORDs;
		std::wstring					mLibraryName;
	};

	class RayGenerationShader : public RayTracingShader
	{
	public:

		RayGenerationShader(std::wstring const& name, std::wstring libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, Type::RayGeneration, name, libraryName) {}
	};

	class MissShader : public RayTracingShader
	{
	public:

		MissShader(std::wstring const& name, std::wstring libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, Type::Miss, name, libraryName) {}
	};

	class ClosestHitShader : public RayTracingShader
	{
	public:

		ClosestHitShader(std::wstring const& name, std::wstring libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, Type::ClosestHit, name, libraryName) {}
	};

	class AnyHitShader : public RayTracingShader
	{
	public:

		AnyHitShader(std::wstring const& name, std::wstring libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, Type::AnyHit, name, libraryName) {}
	};

	class IntersectionShader : public RayTracingShader
	{
	public:

		IntersectionShader(std::wstring const& name, std::wstring libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, Type::Intersection, name, libraryName) {}
	};

	struct RayTracingHitGroup
	{
		D3D12_HIT_GROUP_TYPE			Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		std::wstring					Name = L"";

		ClosestHitShader*				ClosestHitShader = nullptr;
		IntersectionShader*				IntersectionShader = nullptr;
		AnyHitShader*					AnyHitShader = nullptr;
	};

}