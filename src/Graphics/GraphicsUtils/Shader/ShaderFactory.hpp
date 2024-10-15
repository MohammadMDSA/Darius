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

		INLINE std::shared_ptr<VertexShader>	CompileVertexShader(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode) { return CompileShaderGeneric<VertexShader>(config, forceRecompile, shaderCode); }
		INLINE std::shared_ptr<DomainShader>	CompileDomainShader(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode) { return CompileShaderGeneric<DomainShader>(config, forceRecompile, shaderCode); }
		INLINE std::shared_ptr<HullShader>		CompileHullShader(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode) { return CompileShaderGeneric<HullShader>(config, forceRecompile, shaderCode); }
		INLINE std::shared_ptr<GeometryShader>	CompileGeometryShader(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode) { return CompileShaderGeneric<GeometryShader>(config, forceRecompile, shaderCode); }
		INLINE std::shared_ptr<PixelShader>		CompilePixelShader(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode) { return CompileShaderGeneric<PixelShader>(config, forceRecompile, shaderCode); }
		INLINE std::shared_ptr<ComputeShader>	CompileComputeShader(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode) { return CompileShaderGeneric<ComputeShader>(config, forceRecompile, shaderCode); }
		INLINE std::shared_ptr<ShaderLibrary>	CompileShaderLibrary(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode) { return CompileShaderGeneric<ShaderLibrary>(config, forceRecompile, shaderCode); }
		INLINE std::shared_ptr<VertexShader>	CompileVertexShaderAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<VertexShader>)> callback) { return CompileShaderGenericAsync<VertexShader>(config, forceRecompile, shaderCode, callback); }
		INLINE std::shared_ptr<DomainShader>	CompileDomainShaderAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<DomainShader>)> callback) { return CompileShaderGenericAsync<DomainShader>(config, forceRecompile, shaderCode, callback); }
		INLINE std::shared_ptr<HullShader>		CompileHullShaderAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<HullShader>)> callback) { return CompileShaderGenericAsync<HullShader>(config, forceRecompile, shaderCode, callback); }
		INLINE std::shared_ptr<GeometryShader>	CompileGeometryShaderAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<GeometryShader>)> callback) { return CompileShaderGenericAsync<GeometryShader>(config, forceRecompile, shaderCode, callback); }
		INLINE std::shared_ptr<PixelShader>		CompilePixelShaderAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<PixelShader>)> callback) { return CompileShaderGenericAsync<PixelShader>(config, forceRecompile, shaderCode, callback); }
		INLINE std::shared_ptr<ComputeShader>	CompileComputeShaderAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<ComputeShader>)> callback) { return CompileShaderGenericAsync<ComputeShader>(config, forceRecompile, shaderCode, callback); }
		INLINE std::shared_ptr<ShaderLibrary>	CompileShaderLibraryAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<ShaderLibrary>)> callback) { return CompileShaderGenericAsync<ShaderLibrary>(config, forceRecompile, shaderCode, callback); }

		std::wstring						GetVertexShaderCompiler() const;
		std::wstring						GetDomainShaderCompiler() const;
		std::wstring						GetHullShaderCompiler() const;
		std::wstring						GetGeometryShaderCompiler() const;
		std::wstring						GetPixelShaderCompiler() const;
		std::wstring						GetComputeShaderCompiler() const;
		std::wstring						GetShaderLibraryCompiler() const;
		std::wstring						GetCompiler(Type::EShaderType type) const;

		static INLINE D_CONTAINERS::DVector<std::wstring>	GetDefaultIncludes() { return sDefaultIncludes; }

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

		bool							CompileShaderInternal(CompiledShader* shader, std::wstring const& compiler, void const* shaderCode, size_t shaderCodeSize, bool library) const;

		template<class SHADER>
		std::shared_ptr<SHADER>			CompileShaderGeneric(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode)
		{
			auto normalPath = config.Path.lexically_normal();
			ShaderCacheKey cacheKey;

			// Finding shader fromc cache
			auto cachedShader = GetShaderFromCache<SHADER>(config, SHADER::GetTypeStatic(), cacheKey);
			if (!forceRecompile && cachedShader)
				return cachedShader;

			std::shared_ptr<SHADER> shader = std::make_shared<SHADER>(GetShaderNameFromPath(normalPath), config);

			Type::EShaderType shaderType = SHADER::GetTypeStatic();
			void const* shaderCodeData = shaderCode ? shaderCode->data() : nullptr;

			bool success = CompileShaderInternal(shader.get(), GetCompiler(shaderType), shaderCode ? shaderCode->data() : nullptr, shaderCode ? shaderCode->size() : 0, shaderType == Type::ShaderLibrary);

			if (success)
			{
				mShaderCache[cacheKey] = shader;
				return shader;
			}

			return nullptr;
		}

		template<class SHADER>
		std::shared_ptr<SHADER>			CompileShaderGenericAsync(ShaderCompileConfig const& config, bool forceRecompile, std::shared_ptr<D_CONTAINERS::DVector<std::byte>> shaderCode, std::function<void(std::shared_ptr<SHADER>)> callback)
		{
			auto normalPath = config.Path.lexically_normal();
			ShaderCacheKey cacheKey;
			auto cachedShader = GetShaderFromCache<SHADER>(config, SHADER::GetTypeStatic(), cacheKey);
			if (!forceRecompile && cachedShader)
				return cachedShader;

			std::shared_ptr<SHADER> shader = std::make_shared<SHADER>(GetShaderNameFromPath(normalPath), config);
			auto compileTask = new AsyncShaderCompileTask<SHADER>();
			compileTask->mShader = shader;
			compileTask->mShaderCode = shaderCode;
			compileTask->mFactory = shared_from_this();
			compileTask->mCacheKey = cacheKey;
			compileTask->mForceRecompile = forceRecompile;
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

		static D_CONTAINERS::DVector<std::wstring>	sDefaultIncludes;
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
			if (mForceRecompile || !mShader->IsCompiled())
			{
				Type::EShaderType shaderType = SHADER::GetTypeStatic();
				bool success = mFactory->CompileShaderInternal(mShader.get(), mFactory->GetCompiler(shaderType), mShaderCode ? mShaderCode->data() : nullptr, mShaderCode ? mShaderCode->size() : 0, shaderType);

				if(success)
					mFactory->mShaderCache[mCacheKey] = mShader;
			}

			if (mCallback)
				mCallback(mShader);

		}

		ShaderPtr								mShader = nullptr;
		std::shared_ptr<ShaderFactory>			mFactory;
		std::shared_ptr<D_CONTAINERS::DVector<std::byte>> mShaderCode = nullptr;
		ShaderCacheKey							mCacheKey;
		bool									mForceRecompile = false;
		std::function<void(ShaderPtr)>			mCallback = nullptr;
	};
}
