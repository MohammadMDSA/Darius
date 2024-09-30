#pragma once

#include <Core/Containers/Map.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Core/Signal.hpp>
#include <Core/StringId.hpp>
#include <Utils/Common.hpp>
#include <Utils/Assert.hpp>

#ifndef D_GRAPHICS_SHADER
#define D_GRAPHICS_SHADERS Darius::Graphics::Utils::Shaders
#endif // !D_GRAPHICS_SHADER


namespace Darius::Graphics::Utils
{
	class RootSignature;
}

namespace Darius::Graphics::Utils::Shaders
{

	namespace Type
	{
		enum EShaderType : UINT16
		{
			Vertex = 0x1,

			//Tesselation
			Domain = 0x2,
			Hull = 0x4,
			TesselationShader = Domain | Hull,

			Geometry = 0x8,

			Pixel = 0x10,

			// Ray Tracing
			RayGeneration = 0x20,
			Miss = 0x40,
			ClosestHit = 0x80,
			AnyHit = 0x100,
			Intersection = 0x200,
			Callable = 0x400,
			RayTracingShader = RayGeneration | Miss | ClosestHit | AnyHit | Intersection | Callable,
			ShaderLibrary = 0x800,
			Compute = 0x1000
		};
	}

	struct ShaderMacro
	{
	public:
		const D_CORE::StringId Name;
		const D_CORE::StringId Definition;

		std::wstring GetRawDefine() const;
	};

	struct ShaderMacroContainer
	{
	public:
		bool AddMacro(ShaderMacro const& macro);

		// Returns the value either updated or inserted
		D_CORE::StringId AddOrModify(ShaderMacro const& macro);
		inline bool HasMacro(D_CORE::StringId const& name) const { return Macros.contains(name); }
		bool TryRemove(D_CORE::StringId const& name) { return Macros.erase(name); }

		D_CONTAINERS::DVector<std::wstring>	GetRawDefines() const;
		D_CONTAINERS::DVector<ShaderMacro>	GetMacros() const;

		bool operator ==(ShaderMacroContainer const& other) const
		{
			return Macros == other.Macros;
		}

	private:
		D_CONTAINERS::DMap<D_CORE::StringId, D_CORE::StringId> Macros;

		friend struct std::hash<ShaderMacroContainer>;

	};

	struct ShaderCompileConfig
	{
		D_CORE::StringId		EntryPoint;
		D_FILE::Path			Path;
		ShaderMacroContainer	Macros;

		bool operator ==(ShaderCompileConfig const& other) const
		{
			return EntryPoint == other.EntryPoint &&
				Macros == other.Macros;
		}
	};

#define ShaderTypeGetter(type) \
	INLINE virtual Type::EShaderType	GetType() const override { return type; } \
	static Type::EShaderType			GetTypeStatic() { return type; }

	class Shader
	{
	public:

		INLINE D_CORE::StringId		GetName() const { return mName; }
		INLINE std::wstring			GetNameWStr() const { return STR2WSTR(mName.string()); }
		virtual Type::EShaderType	GetType() const = 0;

	protected:
		Shader(D_CORE::StringId const& name) :
			mName(name) {}

	private:
		const D_CORE::StringId		mName;
	};

	class CompiledShader : public Shader
	{
	public:

		ShaderCompileConfig			GetCompileConfig() const { return mCompileConfig; }
		std::string					GetCompileLog() const { return mCompileLog; }
		ID3D12ShaderReflection*		GetReflectionData() const { return mShaderReflectionData.Get(); }
		ID3DBlob*					GetBinary() const { return mBinary.Get(); }
		bool						IsCompiled() const { return mBinary != nullptr; }
		bool						IsValid() const { return IsCompiled(); }

		NODISCARD
		D_CORE::SignalConnection	SubscribeOnCompiled(std::function<void(CompiledShader*)> callback)
		{
			return mOnCompileSignal.connect(callback);
		}

	protected:
		CompiledShader(D_CORE::StringId const& name, ShaderCompileConfig const& config) :
			Shader(name),
			mCompileConfig(config),
			mCompileLog(""),
			mBinary(nullptr)
		{}

		INLINE virtual void			OnPostCompile()
		{
			mOnCompileSignal(this);
		}

	private:
		const ShaderCompileConfig						mCompileConfig;
		std::string										mCompileLog;
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection>	mShaderReflectionData;
		Microsoft::WRL::ComPtr<ID3DBlob>				mBinary;
		D_CORE::Signal<void(CompiledShader*)>			mOnCompileSignal;

		friend class ShaderFactory;
	};

	class VertexShader : public CompiledShader
	{
	public:

		VertexShader(D_CORE::StringId const& name, ShaderCompileConfig const& config) : CompiledShader(name, config) {}
		ShaderTypeGetter(Type::Vertex);
	};

	class DomainShader : public CompiledShader
	{
	public:

		DomainShader(D_CORE::StringId const& name, ShaderCompileConfig const& config) : CompiledShader(name, config) {}
		ShaderTypeGetter(Type::Domain)
	};

	class HullShader : public CompiledShader
	{
	public:

		HullShader(D_CORE::StringId const& name, ShaderCompileConfig const& config) : CompiledShader(name, config) {}
		ShaderTypeGetter(Type::Hull)
	};

	class GeometryShader : public CompiledShader
	{
	public:

		GeometryShader(D_CORE::StringId const& name, ShaderCompileConfig const& config) : CompiledShader(name, config) {}
		ShaderTypeGetter(Type::Geometry)
	};

	class PixelShader : public CompiledShader
	{
	public:

		PixelShader(D_CORE::StringId const& name, ShaderCompileConfig const& config) : CompiledShader(name, config) {}
		ShaderTypeGetter(Type::Pixel)
	};

	class ComputeShader : public CompiledShader
	{
	public:
		ComputeShader(D_CORE::StringId const& name, ShaderCompileConfig const& config) :
			CompiledShader(name, config) {};

		ShaderTypeGetter(Type::Compute);

		INLINE uint32_t			GetThreadGroupSizeX() const
		{
			auto refData = GetReflectionData();
			if (!refData)
				return 0;

			uint32_t result;
			refData->GetThreadGroupSize(&result, nullptr, nullptr);

			return result;
		}

		INLINE uint32_t			GetThreadGroupSizeY() const
		{
			auto refData = GetReflectionData();
			if (!refData)
				return 0;

			uint32_t result;
			refData->GetThreadGroupSize(nullptr, &result, nullptr);

			return result;
		}

		INLINE uint32_t			GetThreadGroupSizeZ() const
		{
			auto refData = GetReflectionData();
			if (!refData)
				return 0;

			uint32_t result;
			refData->GetThreadGroupSize(nullptr, nullptr, &result);

			return result;
		}

		INLINE void				GetThreadGroupSize(uint32_t* sizeX, uint32_t* sizeY, uint32_t* sizeZ)
		{
			auto refData = GetReflectionData();
			if (!refData)
			{
				sizeX = sizeY = sizeZ = 0;
				return;
			}
			refData->GetThreadGroupSize(sizeX, sizeY, sizeZ);
		}
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

	class ShaderLibrary : public CompiledShader
	{
	public:
		ShaderLibrary(D_CORE::StringId const& name, ShaderCompileConfig const& config) :
			CompiledShader(name, config) {}

		ShaderTypeGetter(Type::ShaderLibrary);

		ID3D12LibraryReflection* GetShaderLibraryReflection() const { return mShaderLibraryReflection.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12LibraryReflection> mShaderLibraryReflection;

		friend class ShaderFactory;
	};

	class RayTracingShader : public Shader
	{
	public:
		INLINE std::shared_ptr<RootSignature>	GetLocalRootSignature() const { return mLocalRootDesc; }
		INLINE UINT32							GetRootParametersSizeInBytes() const { return mLocalRootParametersSizeInDWORDs * 4; }
		INLINE UINT8							GetRootParametersSizeInDWORDs() const { return mLocalRootParametersSizeInDWORDs; }
		INLINE D_CORE::StringId const&			GetLibraryName() const { return mLibraryName; }
		INLINE std::wstring						GetLibraryNameWStr() const { return STR2WSTR(mLibraryName.string()); }

	protected:
		RayTracingShader(std::shared_ptr<RootSignature> localRT, D_CORE::StringId const& name, D_CORE::StringId const& libraryName);

	private:
		std::shared_ptr<RootSignature>	mLocalRootDesc;
		UINT8							mLocalRootParametersSizeInDWORDs;
		D_CORE::StringId				mLibraryName;
	};

	struct IdentifierOwner
	{
	public:
		ShaderIdentifier				Identifier;
	};

	class RayGenerationShader : public RayTracingShader, public IdentifierOwner
	{
	public:

		RayGenerationShader(D_CORE::StringId const& name, D_CORE::StringId const& libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, name, libraryName) {}

		ShaderTypeGetter(Type::RayGeneration)
	};

	class MissShader : public RayTracingShader, public IdentifierOwner
	{
	public:

		MissShader(D_CORE::StringId const& name, D_CORE::StringId const& libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, name, libraryName) {}

		ShaderTypeGetter(Type::Miss)
	};

	class ClosestHitShader : public RayTracingShader
	{
	public:

		ClosestHitShader(D_CORE::StringId const& name, D_CORE::StringId const& libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, name, libraryName) {}

		ShaderTypeGetter(Type::ClosestHit)
	};

	class AnyHitShader : public RayTracingShader
	{
	public:

		AnyHitShader(D_CORE::StringId const& name, D_CORE::StringId const& libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, name, libraryName) {}

		ShaderTypeGetter(Type::AnyHit)
	};

	class IntersectionShader : public RayTracingShader
	{
	public:

		IntersectionShader(D_CORE::StringId const& name, D_CORE::StringId const& libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, name, libraryName) {}

		ShaderTypeGetter(Type::Intersection)
	};

	class CallableShader : public RayTracingShader, public IdentifierOwner
	{
	public:
		CallableShader(D_CORE::StringId const& name, D_CORE::StringId const& libraryName, std::shared_ptr<RootSignature> localRT) : RayTracingShader(localRT, name, libraryName) {}

		ShaderTypeGetter(Type::Callable)
	};

	struct RayTracingHitGroup : public IdentifierOwner
	{
		D3D12_HIT_GROUP_TYPE				Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		std::wstring						Name = L"";

		std::shared_ptr<ClosestHitShader>	ClosestHitShader = nullptr;
		std::shared_ptr<IntersectionShader>	IntersectionShader = nullptr;
		std::shared_ptr<AnyHitShader>		AnyHitShader = nullptr;
	};

#undef ShaderTypeGetter

}
