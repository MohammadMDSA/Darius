#include "Graphics/pch.hpp"
#include "Shaders.hpp"

#include "Graphics/GraphicsUtils/RootSignature.hpp"

namespace Darius::Graphics::Utils::Shaders
{

	const ShaderIdentifier ShaderIdentifier::Null = { 0, 0, 0, 0 };

	RayTracingShader::RayTracingShader(std::shared_ptr<RootSignature> localRT, Type::EShaderType type, std::wstring const& name, std::wstring libraryName) :
		Shader(type, name),
		mLocalRootDesc(localRT),
		mLibraryName(libraryName)
	{
		D_ASSERT(type | Type::RayTracingShader);
		D_ASSERT(localRT->IsFinalized());
		mLocalRootParametersSizeInDWORDs = mLocalRootDesc->GetRootParametersSizeInDWORDs();
	}

}
