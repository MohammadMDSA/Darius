#include "Graphics/pch.hpp"
#include "ShaderFactory.hpp"

#include "Graphics/GraphicsDeviceManager.hpp"
#include "ShaderCompiler.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <sstream>

namespace Darius::Graphics::Utils::Shaders
{
	
	D_CONTAINERS::DVector<std::wstring> ShaderFactory::sDefaultIncludes = { L"Shaders" };


	ShaderFactory::ShaderFactory()
	{
		mShaderModel = D_GRAPHICS_DEVICE::GetFeatureSupport().HighestShaderModel();

		std::wstringstream ss;
		ss << mShaderModel / 16;
		ss << "_";
		ss << mShaderModel % 16;
		mShaderModelStr = ss.str();
	}

	std::wstring ShaderFactory::GetVertexShaderCompiler() const
	{
		std::wstringstream ss;
		ss << "vs_";
		ss << mShaderModelStr;
		return ss.str();
	}

	std::wstring ShaderFactory::GetDomainShaderCompiler() const
	{
		std::wstringstream ss;
		ss << "ds_";
		ss << mShaderModelStr;
		return ss.str();
	}

	std::wstring ShaderFactory::GetHullShaderCompiler() const
	{
		std::wstringstream ss;
		ss << "hs_";
		ss << mShaderModelStr;
		return ss.str();
	}

	std::wstring ShaderFactory::GetGeometryShaderCompiler() const
	{
		std::wstringstream ss;
		ss << "gs_";
		ss << mShaderModelStr;
		return ss.str();
	}

	std::wstring ShaderFactory::GetPixelShaderCompiler() const
	{
		std::wstringstream ss;
		ss << "ps_";
		ss << mShaderModelStr;
		return ss.str();
	}

	std::wstring ShaderFactory::GetComputeShaderCompiler() const
	{
		std::wstringstream ss;
		ss << "cs_";
		ss << mShaderModelStr;
		return ss.str();
	}

	std::wstring ShaderFactory::GetShaderLibraryCompiler() const
	{
		std::wstringstream ss;
		ss << "lib_";
		ss << mShaderModelStr;

		return ss.str();
	}

	bool ShaderFactory::CompileShaderInternal(CompiledShader* shader, std::wstring const& compiler, void const* shaderCode, size_t shaderCodeSize, bool library) const
	{
		if (shader->IsCompiled())
			return shader->IsValid();

		auto const& config = shader->mCompileConfig;
		
		std::wstringstream entryPointSS;
		entryPointSS << config.EntryPoint.string();

		if (library)
		{
			ShaderLibrary* library = static_cast<ShaderLibrary*>(shader);
			shader->mBinary = CompileShader(config.Path, entryPointSS.str(), compiler, shaderCode, shaderCodeSize, config.Macros.GetRawDefines(), sDefaultIncludes, nullptr, library->mShaderLibraryReflection.ReleaseAndGetAddressOf(), shader->mCompileLog, config.ForceDisableOptimization);
		}
		else
		{
			shader->mBinary = CompileShader(config.Path, entryPointSS.str(), compiler, shaderCode, shaderCodeSize, config.Macros.GetRawDefines(), sDefaultIncludes, shader->mShaderReflectionData.ReleaseAndGetAddressOf(), nullptr, shader->mCompileLog, config.ForceDisableOptimization);
		}

		if (shader->IsCompiled())
			shader->OnPostCompile();

		return shader->IsValid();
	}

	D_CORE::StringId ShaderFactory::GetShaderNameFromPath(D_FILE::Path const& path)
	{
		auto shaderNameWstr = D_FILE::GetFileName(path);
		auto shaderNameStr = WSTR2STR(shaderNameWstr);
		return D_CORE::StringId(shaderNameStr.c_str());
	}


	std::wstring ShaderFactory::GetCompiler(Type::EShaderType type) const
	{
		switch (type)
		{
		case Darius::Graphics::Utils::Shaders::Type::Vertex:
			return GetVertexShaderCompiler();
		case Darius::Graphics::Utils::Shaders::Type::Domain:
			return GetDomainShaderCompiler();
		case Darius::Graphics::Utils::Shaders::Type::Hull:
			return GetHullShaderCompiler();
		case Darius::Graphics::Utils::Shaders::Type::Geometry:
			return GetGeometryShaderCompiler();
		case Darius::Graphics::Utils::Shaders::Type::Pixel:
			return GetPixelShaderCompiler();
		case Darius::Graphics::Utils::Shaders::Type::ShaderLibrary:
			return GetShaderLibraryCompiler();
		case Darius::Graphics::Utils::Shaders::Type::Compute:
			return GetComputeShaderCompiler();
		default:
			D_ASSERT_NOENTRY();
			return L"";
		}
	}

}
