#pragma once

#include "Shaders.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Hash.hpp>
#include <Job/Job.hpp>

#ifndef D_GRAPHICS_SHADER
#define D_GRAPHICS_SHADERS Darius::Graphics::Utils::Shaders
#endif // !D_GRAPHICS_SHADER

namespace Darius::Graphics::Utils::Shaders
{
	struct ShaderCacheKey
	{
		ShaderCompileConfig	Config;
		D_CORE::StringId	PathId;
		Type::EShaderType	ShaderType;

		bool operator ==(ShaderCacheKey const& other) const
		{
			return Config == other.Config &&
				PathId == other.PathId &&
				ShaderType == other.ShaderType;
		}

	};
}

namespace std
{
	template<>
	struct hash<D_GRAPHICS_SHADERS::ShaderCacheKey>
	{
		inline size_t operator () (D_GRAPHICS_SHADERS::ShaderCacheKey const& key) const
		{
			size_t seed = 0;
			seed = std::hash<D_CORE::StringId>()(key.PathId);
			return seed ^ (size_t)key.ShaderType;

		}
	};
}

namespace Darius::Graphics::Utils::Shaders
{
	template<class SHADER>
	struct AsyncShaderCompileTask;

	class ShaderFactory : public std::enable_shared_from_this<ShaderFactory>
	{
	public:
		ShaderFactory();

		INLINE std::shared_ptr<VertexShader>	CompileVertexShader(ShaderCompileConfig const& config) { return CompileShaderGeneric<VertexShader>(config); }
		INLINE std::shared_ptr<DomainShader>	CompileDomainShader(ShaderCompileConfig const& config)  { return CompileShaderGeneric<DomainShader>(config); }
		INLINE std::shared_ptr<HullShader>		CompileHullShader(ShaderCompileConfig const& config)  { return CompileShaderGeneric<HullShader>(config); }
		INLINE std::shared_ptr<GeometryShader>	CompileGeometryShader(ShaderCompileConfig const& config)  { return CompileShaderGeneric<GeometryShader>(config); }
		INLINE std::shared_ptr<PixelShader>		CompilePixelShader(ShaderCompileConfig const& config)  { return CompileShaderGeneric<PixelShader>(config); }
		INLINE std::shared_ptr<ComputeShader>	CompileComputeShader(ShaderCompileConfig const& config)  { return CompileShaderGeneric<ComputeShader>(config); }
		INLINE std::shared_ptr<ShaderLibrary>	CompileShaderLibrary(ShaderCompileConfig const& config)  { return CompileShaderGeneric<ShaderLibrary>(config); }
		INLINE std::shared_ptr<VertexShader>	CompileVertexShaderAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<VertexShader>)> callback) { return CompileShaderGenericAsync<VertexShader>(config, callback); }
		INLINE std::shared_ptr<DomainShader>	CompileDomainShaderAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<DomainShader>)> callback) { return CompileShaderGenericAsync<DomainShader>(config, callback); }
		INLINE std::shared_ptr<HullShader>		CompileHullShaderAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<HullShader>)> callback) { return CompileShaderGenericAsync<HullShader>(config, callback); }
		INLINE std::shared_ptr<GeometryShader>	CompileGeometryShaderAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<GeometryShader>)> callback) { return CompileShaderGenericAsync<GeometryShader>(config, callback); }
		INLINE std::shared_ptr<PixelShader>		CompilePixelShaderAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<PixelShader>)> callback) { return CompileShaderGenericAsync<PixelShader>(config, callback); }
		INLINE std::shared_ptr<ComputeShader>	CompileComputeShaderAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<ComputeShader>)> callback) { return CompileShaderGenericAsync<ComputeShader>(config, callback); }
		INLINE std::shared_ptr<ShaderLibrary>	CompileShaderLibraryAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<ShaderLibrary>)> callback) { return CompileShaderGenericAsync<ShaderLibrary>(config, callback); }

		std::wstring						GetVertexShaderCompiler() const;
		std::wstring						GetDomainShaderCompiler() const;
		std::wstring						GetHullShaderCompiler() const;
		std::wstring						GetGeometryShaderCompiler() const;
		std::wstring						GetPixelShaderCompiler() const;
		std::wstring						GetComputeShaderCompiler() const;
		std::wstring						GetShaderLibraryCompiler() const;
		std::wstring						GetCompiler(Type::EShaderType type) const;

		template<typename V>
		std::shared_ptr<V>					GetShaderFromCache(ShaderCompileConfig const& config, Type::EShaderType shaderType, ShaderCacheKey& cacheKey)
		{
			auto normalPath = config.Path.lexically_normal();
			cacheKey =
			{
				.Config = config,
				.PathId = Darius::Core::StringId(normalPath.string().c_str()),
				.ShaderType = shaderType
			};

			auto cacheSearch = mShaderCache.find(cacheKey);
			if (cacheSearch != mShaderCache.end())
			{
				auto ptr = cacheSearch->second;
				if (!ptr)
					mShaderCache.erase(cacheKey);
				else
				{
					return std::static_pointer_cast<V>(ptr);
				}
			}

			return nullptr;
		}

	private:

		bool							CompileShaderInternal(CompiledShader* shader, std::wstring const& compiler, bool library) const;

		template<class SHADER>
		std::shared_ptr<SHADER>			CompileShaderGeneric(ShaderCompileConfig const& config)
		{
			auto normalPath = config.Path.lexically_normal();
			ShaderCacheKey cacheKey;
			auto cachedShader = GetShaderFromCache<SHADER>(config, SHADER::GetTypeStatic(), cacheKey);
			if (cachedShader)
				return cachedShader;

			std::shared_ptr<SHADER> shader = std::make_shared<SHADER>(GetShaderNameFromPath(normalPath), config);

			Type::EShaderType shaderType = SHADER::GetTypeStatic();
			CompileShaderInternal(shader.get(), GetCompiler(shaderType), shaderType == Type::ShaderLibrary);

			mShaderCache[cacheKey] = shader;
			return shader;
		}

		template<class SHADER>			
		std::shared_ptr<SHADER>			CompileShaderGenericAsync(ShaderCompileConfig const& config, std::function<void(std::shared_ptr<SHADER>)> callback)
		{
			auto normalPath = config.Path.lexically_normal();
			ShaderCacheKey cacheKey;
			auto cachedShader = GetShaderFromCache<SHADER>(config, SHADER::GetTypeStatic(), cacheKey);
			if (cachedShader)
				return cachedShader;

			std::shared_ptr<SHADER> shader = std::make_shared<SHADER>(GetShaderNameFromPath(normalPath), config);
			auto compileTask = new AsyncShaderCompileTask<SHADER>();
			compileTask->mShader = shader;
			compileTask->mFactory = shared_from_this();
			compileTask->mCallback = callback;

			D_JOB::AddPinnedTask(compileTask, D_JOB::ThreadType::FileIO);
		}

		static D_CORE::StringId			GetShaderNameFromPath(D_FILE::Path const& path);


		D3D_SHADER_MODEL	mShaderModel;
		std::wstring		mShaderModelStr;

		D_CONTAINERS::DUnorderedMap<ShaderCacheKey, std::shared_ptr<CompiledShader>, std::hash<ShaderCacheKey>> mShaderCache;

		friend AsyncShaderCompileTask<VertexShader>;
		friend AsyncShaderCompileTask<DomainShader>;
		friend AsyncShaderCompileTask<HullShader>;
		friend AsyncShaderCompileTask<GeometryShader>;
		friend AsyncShaderCompileTask<PixelShader>;
		friend AsyncShaderCompileTask<ComputeShader>;
		friend AsyncShaderCompileTask<ShaderLibrary>;
	};

	template<class SHADER>
	struct AsyncShaderCompileTask : public D_JOB::IPinnedTask
	{
		typedef std::shared_ptr<SHADER> ShaderPtr;

		virtual void Execute() override
		{
			if (!mShader)
			{
				D_LOG_WARN("No shader provided to compile task. Ignoring callback...");
				return;
			}

			auto normalPath = mShader->GetCompileConfig().Path.lexically_normal();
			if (!mShader->IsCompiled())
			{
				Type::EShaderType shaderType = SHADER::GetTypeStatic();
				mFactory->CompileShaderInternal(mShader.get(), mFactory->GetCompiler(shaderType), shaderType);
			}

			if (mCallback)
				mCallback(mShader);

		}

		ShaderPtr								mShader = nullptr;
		std::shared_ptr<const ShaderFactory>	mFactory;
		std::function<void(ShaderPtr)>			mCallback = nullptr;
	};
}
