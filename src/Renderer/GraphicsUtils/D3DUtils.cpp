#include "./Renderer/pch.hpp"
#include "D3DUtils.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Utils/Log.hpp>

#include <dxcapi.h>         // Be sure to link with dxcompiler.lib.
#include <d3d12shader.h>    // Shader reflection.

using namespace Microsoft::WRL;

namespace Darius::Graphics::Utils
{

	ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const std::wstring& entrypoint, const std::wstring& target)
	{
        // 
    // Create compiler and utils.
    //
        ComPtr<IDxcUtils> pUtils;
        ComPtr<IDxcCompiler3> pCompiler;
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

        //
        // Create default include handler. (You can create your own...)
        //
        ComPtr<IDxcIncludeHandler> pIncludeHandler;
        pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);


        //
        // COMMAND LINE:
        // dxc myshader.hlsl -E main -T ps_6_0 -Zi -D MYDEFINE=1 -Fo myshader.bin -Fd myshader.pdb -Qstrip_reflect
        //
        auto pdbName = filename + L".pdb";

        LPCWSTR pszArgs[] =
        {
            filename.c_str(),            // Optional shader source file name for error reporting
            // and for PIX shader source view.  
            L"-E", entrypoint.c_str(),   // Entry point.
            L"-T", target.c_str(),       // Target.
            L"-Fd", pdbName.c_str(),     // The file name of the pdb. This must either be supplied
            // or the autogenerated file name must be used.
            L"-Zpc",
#ifdef _DEBUG
            L"-Od",                      // Disable optimizations
            L"-Zi",                       // Enable debug information
            //L"-Zs",                      // Enable debug information (slim format)
#endif
        };


        //
        // Open source file.  
        //
        ComPtr<IDxcBlobEncoding> pSource = nullptr;
        pUtils->LoadFile(filename.c_str(), nullptr, &pSource);
        DxcBuffer Source;
        Source.Ptr = pSource->GetBufferPointer();
        Source.Size = pSource->GetBufferSize();
        Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.


        //
        // Compile it with specified arguments.
        //
        ComPtr<IDxcResult> pResults;
        pCompiler->Compile(
            &Source,                // Source buffer.
            pszArgs,                // Array of pointers to arguments.
            _countof(pszArgs),      // Number of arguments.
            pIncludeHandler.Get(),        // User-provided interface to handle #include directives (optional).
            IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
        );

        // Fetching the hResult
        HRESULT hrStatus;
        pResults->GetStatus(&hrStatus);

        //
        // Print errors if present.
        //
        ComPtr<IDxcBlobUtf8> pErrors = nullptr;
        pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
        // Note that d3dcompiler would return null if no errors or warnings are present.
        // IDxcCompiler3::Compile will always return an error buffer, but its length
        // will be zero if there are no warnings or errors.
        if (pErrors != nullptr && pErrors->GetStringLength() != 0)
        {
            char* msg = (char*)pErrors->GetBufferPointer();
            OutputDebugString(msg);
            if (D_HR_SUCCEEDED(hrStatus))
                D_LOG_WARN(msg);
            else
                D_LOG_ERROR(msg);
        }

        //
        // Quit if the compilation failed.
        //

        D_HR_CHECK(hrStatus);


        //
        // Save shader binary.
        //
        ComPtr<IDxcBlob> pShader = nullptr;
        ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
        pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);


        //
        // Save pdb.
        //
        ComPtr<IDxcBlob> pPDB = nullptr;
        ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
        pResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
        {
            FILE* fp = NULL;

            std::wstring name = pPDBName->GetStringPointer();

            // Note that if you don't specify -Fd, a pdb name will be automatically generated.
            // Use this file name to save the pdb so that PIX can find it quickly.
            _wfopen_s(&fp, pPDBName->GetStringPointer(), L"wb");
            fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp);
            fclose(fp);
        }


        return ComPtr<ID3DBlob>(reinterpret_cast<ID3DBlob*>(pShader.Get()));
	}

}