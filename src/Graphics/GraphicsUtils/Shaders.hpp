#pragma once

#include "RootSignature.hpp"

#include <Utils/Common.hpp>

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
			Callable			= 0x400,
			RayTracingShader	= RayGeneration | Miss | ClosestHit | AnyHit | Intersection | Callable

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

	struct ShaderIdentifier
	{
		UINT64 Data[4] = { ~0ull, ~0ull, ~0ull, ~0ull };

		// No shader is executed if a shader table record with null identifier is encountered.
		static const ShaderIdentifier Null;

		bool operator == (const ShaderIdentifier& other) const
		{
			return Data[0] == other.Data[0]
				&& Data[1] == other.Data[1]
				&& Data[2] == other.Data[2]
				&& Data[3] == other.Data[3];
		}

		bool operator != (const ShaderIdentifier& other) const
		{
			return !(*this == other);
		}

		bool IsValid() const
		{
			return *this != ShaderIdentifier();
		}

		void SetData(const void* inData)
		{
			std::memcpy(Data, inData, sizeof(Data));
		}
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

	struct IdentifierOwner
	{
	public:
		ShaderIdentifier				Identifier;
	};

	class RayGenerationShader : public RayTracingShader, public IdentifierOwner
	{
	public:

		RayGenerationShader(std::wstring const& name, std::wstring libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, Type::RayGeneration, name, libraryName) {}
	};

	class MissShader : public RayTracingShader, public IdentifierOwner
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

	class CallableShader : public RayTracingShader, public IdentifierOwner
	{
	public:
		CallableShader(std::wstring const& name, std::wstring libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, Type::Callable, name, libraryName) {}
	};

	struct RayTracingHitGroup : public IdentifierOwner
	{
		D3D12_HIT_GROUP_TYPE				Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		std::wstring						Name = L"";

		std::shared_ptr<ClosestHitShader>	ClosestHitShader = nullptr;
		std::shared_ptr<IntersectionShader>	IntersectionShader = nullptr;
		std::shared_ptr<AnyHitShader>		AnyHitShader = nullptr;
	};

}