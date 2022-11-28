#include "./Renderer/pch.hpp"
#include "D3DUtils.hpp"

#include <Utils/Log.hpp>

using namespace Microsoft::WRL;

namespace Darius::Renderer::GraphicsUtils
{

	ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
	{
		// Use debug flags in debug mode.
		UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hr = S_OK;

		ComPtr<ID3DBlob> byteCode = nullptr;
		ComPtr<ID3DBlob> errors;

		hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

		// Output errors to debug window.
		if (errors != nullptr)
		{
			char* msg = (char*)errors->GetBufferPointer();
			OutputDebugString(msg);
			if (D_HR_SUCCEEDED(hr))
				D_LOG_WARN(msg);
			else
				D_LOG_ERROR(msg);
		}

		D_HR_CHECK(hr);

		return byteCode;
	}

}