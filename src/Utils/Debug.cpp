#include "Debug.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <shlobj.h>
#include <tchar.h>

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif


namespace Darius::Utils::Debug
{
#ifdef _DEBUG

	std::string GetLatestWinPixGpuCapturerPath()
	{
		LPWSTR programFilesPath = nullptr;
		SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

		std::filesystem::path pixInstallationPath = programFilesPath;
		pixInstallationPath /= "Microsoft PIX";

		std::wstring newestVersionFound;

		for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
		{
			if (directory_entry.is_directory())
			{
				if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
				{
					newestVersionFound = directory_entry.path().filename().c_str();
				}
			}
		}

		if (newestVersionFound.empty())
		{
			// TODO: Error, no PIX installation found
		}
		return (pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll").string();
	}

	void AttachWinPixGpuCapturer()
	{
		// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
		// This may happen if the application is launched through the PIX UI. 
		if (GetModuleHandle(_T("WinPixGpuCapturer.dll")) == 0)
		{
			LoadLibrary(_T(GetLatestWinPixGpuCapturerPath().c_str()));
		}
	}

#endif
}