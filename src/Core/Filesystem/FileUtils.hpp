#pragma once

#include "Path.hpp"

#include <fstream>
#include <cstddef>
#include <memory>

#pragma warning(push)
#pragma warning(disable: 4355)
#include <ppl.h>
#include <ppltasks.h>
#pragma warning(pop)

#ifndef D_FILE
#define D_FILE Darius::Core::Filesystem
#endif // !D_FILE

using namespace std;

namespace Darius::Core::Filesystem
{
	typedef shared_ptr<vector<std::byte>> ByteArray;
	static const ByteArray NullFile = std::make_shared<vector<std::byte> >(vector<std::byte>());

	std::wstring GetNewFileName(std::wstring const& baseName, std::wstring const& extension, Path parent);

	ByteArray ReadFileHelper(const wstring& fileName);

	ByteArray ReadFileSync(std::wstring const& path);

	Concurrency::task<ByteArray> ReadFileAsync(std::wstring const& path);

	std::wstring GetFileName(Path path);
}
