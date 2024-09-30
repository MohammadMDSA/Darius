#include "Graphics/pch.hpp"
#include "Shaders.hpp"

#include <sstream>

#include "Graphics/GraphicsUtils/RootSignature.hpp"

namespace Darius::Graphics::Utils::Shaders
{

	std::wstring ShaderMacro::GetRawDefine() const
	{
		std::wstringstream ss;
		ss << Name.string();
		ss << "=";
		ss << Definition.string();
		return ss.str();
	}

	D_CONTAINERS::DVector<ShaderMacro> ShaderMacroContainer::GetMacros() const
	{
		D_CONTAINERS::DVector<ShaderMacro> result;
		for (auto [k, v] : Macros)
		{
			result.push_back({ k, v });
		}
		return result;
	}

	D_CONTAINERS::DVector<std::wstring> ShaderMacroContainer::GetRawDefines() const
	{
		D_CONTAINERS::DVector<std::wstring> result;
		for (auto [k, v] : Macros)
		{
			ShaderMacro macro = { k, v };
			result.push_back(macro.GetRawDefine());
		}

		return result;
	}

	bool ShaderMacroContainer::AddMacro(ShaderMacro const& macro)
	{
		if (Macros.contains(macro.Name))
			return false;

		Macros.emplace(macro.Name, macro.Definition);
		return true;
	}

	D_CORE::StringId ShaderMacroContainer::AddOrModify(ShaderMacro const& macro)
	{
		return Macros.insert_or_assign(macro.Name, macro.Definition).first->second;
	}

	const ShaderIdentifier ShaderIdentifier::Null = { 0, 0, 0, 0 };

	RayTracingShader::RayTracingShader(std::shared_ptr<RootSignature> localRT, D_CORE::StringId const& name, D_CORE::StringId const& libraryName) :
		Shader(name),
		mLocalRootDesc(localRT),
		mLibraryName(libraryName)
	{
		D_ASSERT(localRT->IsFinalized());
		mLocalRootParametersSizeInDWORDs = mLocalRootDesc->GetRootParametersSizeInDWORDs();
	}

}
