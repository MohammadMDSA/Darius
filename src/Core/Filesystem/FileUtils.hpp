#pragma once

#include "Path.hpp"
#include "Core/Serialization/Json.hpp"

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

namespace Darius::Core::Filesystem
{
	typedef std::shared_ptr<std::vector<std::byte>> ByteArray;
	static const ByteArray NullFile = std::make_shared<std::vector<std::byte> >(std::vector<std::byte>());

	std::wstring					GetNewFileName(std::wstring const& baseName, std::wstring const& extension, Path parent);

	ByteArray						ReadFileHelper(const std::wstring& fileName);

	ByteArray						ReadFileSync(std::wstring const& path);

	Concurrency::task<ByteArray>	ReadFileAsync(std::wstring const& path);

	std::wstring					GetFileName(Path path);

	// Calls the callback providing entry path and whether it is a directory
	void							VisitEntriesInDirectory(Path const& path, bool recursively, std::function<void(Path const&, bool)> callback);
	void							VisitFilesInDirectory(Path const& path, bool recursively, std::function<void(Path const&)> callback);

	bool							ReadJsonFile(Path const& filePath, D_SERIALIZATION::Json& json);
	bool							WriteJsonFile(Path const& filePath, D_SERIALIZATION::Json const& json);

}
