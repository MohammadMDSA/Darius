#pragma once

#include "StaticMeshResource.hpp"

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_RENDERER_GEOMETRY;
using namespace D_CORE;

namespace Darius::ResourceManager
{
	class DResourceManager;

	class BatchResource : public StaticMeshResource
	{
		D_CH_RESOURCE_BODY(BatchResource, "Batch", "")

	private:
		friend class DResourceManager;

		BatchResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			StaticMeshResource(uuid, path, name, id, isDefault) {}

	};

}