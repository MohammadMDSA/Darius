#include "Graphics/pch.hpp"
#include "D3DUtils.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Utils/Log.hpp>

#include <dxcapi.h>         // Be sure to link with dxcompiler.lib.
#include <d3d12shader.h>    // Shader reflection.

#include <fstream>

using namespace Microsoft::WRL;

#define SHADER_REFLECTION

namespace Darius::Graphics::Utils
{
    class DxcDllSupport {
    protected:
        HMODULE m_dll;
        DxcCreateInstanceProc m_createFn;
        DxcCreateInstance2Proc m_createFn2;

        HRESULT InitializeInternal(LPCWSTR dllName, LPCSTR fnName) {
            if (m_dll != nullptr) return S_OK;
            m_dll = LoadLibraryW(dllName);

            if (m_dll == nullptr) return HRESULT_FROM_WIN32(GetLastError());
            m_createFn = (DxcCreateInstanceProc)GetProcAddress(m_dll, fnName);

            if (m_createFn == nullptr) {
                HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
                FreeLibrary(m_dll);
                m_dll = nullptr;
                return hr;
            }

            // Only basic functions used to avoid requiring additional headers.
            m_createFn2 = nullptr;
            char fnName2[128];
            size_t s = strlen(fnName);
            if (s < sizeof(fnName2) - 2) {
                memcpy(fnName2, fnName, s);
                fnName2[s] = '2';
                fnName2[s + 1] = '\0';
                m_createFn2 = (DxcCreateInstance2Proc)GetProcAddress(m_dll, fnName2);
            }

            return S_OK;
        }

    public:
        DxcDllSupport() : m_dll(nullptr), m_createFn(nullptr), m_createFn2(nullptr) {
        }

        DxcDllSupport(DxcDllSupport&& other) {
            m_dll = other.m_dll; other.m_dll = nullptr;
            m_createFn = other.m_createFn; other.m_createFn = nullptr;
            m_createFn2 = other.m_createFn2; other.m_createFn2 = nullptr;
        }

        ~DxcDllSupport() {
            Cleanup();
        }

        HRESULT Initialize() {
            return InitializeInternal(L"dxcompiler.dll", "DxcCreateInstance");
        }

        HRESULT InitializeForDll(_In_z_ const wchar_t* dll, _In_z_ const char* entryPoint) {
            return InitializeInternal(dll, entryPoint);
        }

        template <typename TInterface>
        HRESULT CreateInstance(REFCLSID clsid, _Outptr_ TInterface** pResult) {
            return CreateInstance(clsid, __uuidof(TInterface), (IUnknown**)pResult);
        }

        HRESULT CreateInstance(REFCLSID clsid, REFIID riid, _Outptr_ IUnknown** pResult) {
            if (pResult == nullptr) return E_POINTER;
            if (m_dll == nullptr) return E_FAIL;
            HRESULT hr = m_createFn(clsid, riid, (LPVOID*)pResult);
            return hr;
        }

        template <typename TInterface>
        HRESULT CreateInstance2(IMalloc* pMalloc, REFCLSID clsid, _Outptr_ TInterface** pResult) {
            return CreateInstance2(pMalloc, clsid, __uuidof(TInterface), (IUnknown**)pResult);
        }

        HRESULT CreateInstance2(IMalloc* pMalloc, REFCLSID clsid, REFIID riid, _Outptr_ IUnknown** pResult) {
            if (pResult == nullptr) return E_POINTER;
            if (m_dll == nullptr) return E_FAIL;
            if (m_createFn2 == nullptr) return E_FAIL;
            HRESULT hr = m_createFn2(pMalloc, clsid, riid, (LPVOID*)pResult);
            return hr;
        }

        bool HasCreateWithMalloc() const {
            return m_createFn2 != nullptr;
        }

        bool IsEnabled() const {
            return m_dll != nullptr;
        }

        void Cleanup() {
            if (m_dll != nullptr) {
                m_createFn = nullptr;
                m_createFn2 = nullptr;
                FreeLibrary(m_dll);
                m_dll = nullptr;
            }
        }

        HMODULE Detach() {
            HMODULE module = m_dll;
            m_dll = nullptr;
            return module;
        }
    };

    static DxcDllSupport gDxcDllHelper;

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

        std::vector<LPCWSTR> pszArgs =
        {
            filename.c_str(),            // Optional shader source file name for error reporting
            // and for PIX shader source view.  
            L"-E", entrypoint.c_str(),  // Entry point.
            L"-T", target.c_str(),      // Target.
            L"-Fd", pdbName.c_str(),    // The file name of the pdb. This must either be supplied
            // or the autogenerated file name must be used.
            L"-Zpc",
#ifdef _DEBUG
            L"-Od",                     // Disable optimizations
            L"-Zi",                     // Enable debug information
            //L"-Zs",                   // Enable debug information (slim format)
#endif
        };

        if (target.starts_with(L"lib"))
            pszArgs.push_back(L"-Vd");


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
            &Source,                    // Source buffer.
            pszArgs.data(),             // Array of pointers to arguments.
            (UINT)pszArgs.size(),       // Number of arguments.
            pIncludeHandler.Get(),      // User-provided interface to handle #include directives (optional).
            IID_PPV_ARGS(&pResults)     // Compiler output status, buffer, and errors.
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

        //
        // Reflection Data
        //

#ifdef SHADER_REFLECTION

        ComPtr<IDxcBlob> pReflectionData;

        pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
        if (pReflectionData != nullptr)
        {
            // Optionally, save reflection blob for later here.

            // Create reflection interface.
            DxcBuffer ReflectionData;
            ReflectionData.Encoding = DXC_CP_ACP;
            ReflectionData.Ptr = pReflectionData->GetBufferPointer();
            ReflectionData.Size = pReflectionData->GetBufferSize();

            ComPtr<ID3D12ShaderReflection> pReflection;
            D_HR_CHECK(pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&pReflection)));

        }
#endif

        return ComPtr<ID3DBlob>(reinterpret_cast<ID3DBlob*>(pShader.Get()));
	}

    template<class BlotType>
    std::string convertBlobToString(BlotType pBlob)
    {
        std::vector<char> infoLog(pBlob->GetBufferSize() + 1);
        memcpy(infoLog.data(), pBlob->GetBufferPointer(), pBlob->GetBufferSize());
        infoLog[pBlob->GetBufferSize()] = 0;
        return std::string(infoLog.data());
    }

    ComPtr<ID3DBlob> CompileLibrary(std::wstring const& filename, std::wstring const& target)
    {
        // Initialize the helper
        D_HR_CHECK(gDxcDllHelper.Initialize());
        ComPtr<IDxcCompiler> pCompiler;
        ComPtr<IDxcLibrary> pLibrary;
        ComPtr<IDxcUtils> pUtils;
        ComPtr<IDxcContainerReflection> pReflection;
        D_HR_CHECK(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)));
        D_HR_CHECK(gDxcDllHelper.CreateInstance(CLSID_DxcCompiler, pCompiler.GetAddressOf()));
        D_HR_CHECK(gDxcDllHelper.CreateInstance(CLSID_DxcLibrary, pLibrary.GetAddressOf()));
        pReflection->

        ComPtr<IDxcIncludeHandler> pIncludeHandler;
        pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

        // Open and read the file
        std::ifstream shaderFile(filename);
        if (shaderFile.good() == false)
        {
            D_LOG_ERROR("Can't open file " + WSTR2STR(filename));
            return nullptr;
        }
        std::stringstream strStream;
        strStream << shaderFile.rdbuf();
        shaderFile.close();
        std::string shader = strStream.str();

        // Create blob from the string
        ComPtr<IDxcBlobEncoding> pTextBlob;
        D_HR_CHECK(pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)shader.c_str(), (uint32_t)shader.size(), 0, &pTextBlob));

        // Compile
        ComPtr<IDxcOperationResult> pResult;
        D_HR_CHECK(pCompiler->Compile(pTextBlob.Get(), filename.c_str(), L"", target.c_str(), nullptr, 0, nullptr, 0, pIncludeHandler.Get(), &pResult));

        // Verify the result
        HRESULT resultCode;
        D_HR_CHECK(pResult->GetStatus(&resultCode));
        if (FAILED(resultCode))
        {
            ComPtr<IDxcBlobEncoding> pError;
            D_HR_CHECK(pResult->GetErrorBuffer(&pError));
            std::string log = convertBlobToString(pError);
            D_LOG_ERROR("Compiler error:\n" + log);
            return nullptr;
        }

  
        // TODO:: Add reflection data extraction for compiled shader libraries
//#ifdef SHADER_REFLECTION
//
//        ComPtr<IDxcBlob> pReflectionData;
//
//        pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
//        if (pReflectionData != nullptr)
//        {
//            // Optionally, save reflection blob for later here.
//
//            // Create reflection interface.
//            DxcBuffer ReflectionData;
//            ReflectionData.Encoding = DXC_CP_ACP;
//            ReflectionData.Ptr = pReflectionData->GetBufferPointer();
//            ReflectionData.Size = pReflectionData->GetBufferSize();
//
//            ComPtr<ID3D12ShaderReflection> pReflection;
//            D_HR_CHECK(pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&pReflection)));
//
//        }
//#endif

        ComPtr<IDxcBlob> pBlob;
        D_HR_CHECK(pResult->GetResult(&pBlob));

        return ComPtr<ID3DBlob>(reinterpret_cast<ID3DBlob*>(pBlob.Get()));
    }
}