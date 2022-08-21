#pragma once

#include "Path.hpp"

#ifndef D_FILE
#define D_FILE Darius::Core::Filesystem
#endif // !D_FILE

namespace Darius::Core::Filesystem
{

	std::wstring GetNewFileName(std::wstring const& baseName, std::wstring const& extension, Path parent)
	{
		short newIndex = -1;

		Path resultPath;
		std::wstring result;
		do
		{
			auto directory = Path(parent);
			newIndex++;
			if (newIndex)
				result = baseName + L"(" + std::to_wstring(newIndex) + L")" + extension;
			else
				result = baseName + extension;
			resultPath = directory.append(result);
		} while (D_H_ENSURE_FILE(resultPath));

		return result;
	}

}
