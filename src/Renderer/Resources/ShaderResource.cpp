#include "Renderer/pch.hpp"
#include "ShaderResource.hpp"

#include <Core/Filesystem/FileUtils.hpp>

#include "ShaderResource.sgenerated.hpp"

namespace Darius::Renderer
{
	bool ShaderResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		return D_FILE::
	}

	void ShaderResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json, bool& dirtyDisk)
	{
		dirtyDisk = false;
		mCode = D_FILE::ReadFileSync(GetPath());
	}
}