#include "Core/pch.hpp"
#include "FileUtils.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>
#include <Utils/Log.hpp>

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

	ByteArray ReadFileHelper(const wstring& fileName)
	{
		struct _stat64 fileStat;
		int fileExists = _wstat64(fileName.c_str(), &fileStat);
		if (fileExists == -1)
			return NullFile;

		ifstream file(fileName, ios::in | ios::binary);
		if (!file)
			return NullFile;

		ByteArray byteArray = std::make_shared<vector<std::byte> >(fileStat.st_size);
		file.read((char*)byteArray->data(), byteArray->size());
		file.close();

		return byteArray;
	}

	ByteArray ReadFileSync(std::wstring const& path)
	{
		return ReadFileHelper(path);
	}

	Concurrency::task<ByteArray> ReadFileAsync(std::wstring const& path)
	{
		return Concurrency::create_task([=] { return ReadFileHelper(path); });
	}

	std::wstring GetFileName(Path path)
	{
		auto ext = path.extension().wstring();
		auto fullname = path.filename().wstring();

		return fullname.substr(0, fullname.length() - ext.length());
	}

	void VisitEntriesInDirectory(Path const& path, bool recursively, std::function<void(Path const&, bool)> callback)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			callback(entry.path(), entry.is_directory());
			if (entry.is_directory() && recursively)
			{
				VisitEntriesInDirectory(entry.path(), true, callback);
			}
		}
	}

	void VisitFilesInDirectory(Path const& path, bool recursively, std::function<void(Path const&)> callback)
	{
		VisitEntriesInDirectory(path, recursively, [&](Path const& _path, bool isDir)
			{
				if (!isDir)
					callback(_path);
			});
	}
}