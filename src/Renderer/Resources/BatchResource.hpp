#pragma once

#include "StaticMeshResource.hpp"

#include "BatchResource.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DResourceManager;

	class DClass(Serialize, Resource) BatchResource : public StaticMeshResource
	{
		D_CH_RESOURCE_BODY(BatchResource, "Batch", "")

	private:
		friend class DResourceManager;

		BatchResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			StaticMeshResource(uuid, path, name, id, isDefault) {}

	public:
		Darius_Graphics_BatchResource_GENERATED

	};

}

File_BatchResource_GENERATED
