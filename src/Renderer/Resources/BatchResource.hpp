#pragma once

#include "StaticMeshResource.hpp"

#include "BatchResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	class DClass(Serialize, Resource) BatchResource : public StaticMeshResource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(BatchResource, "Batch", "")

	private:
		friend class DResourceManager;

		BatchResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource * parent, bool isDefault = false) :
			StaticMeshResource(uuid, path, name, id, parent, isDefault) {}


	};

}

File_BatchResource_GENERATED
