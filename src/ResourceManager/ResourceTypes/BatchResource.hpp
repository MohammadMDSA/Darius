#pragma once

#include "MeshResource.hpp"

#include <Renderer/Geometry/MeshData.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/GraphicsUtils/VertexTypes.hpp>


#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_RENDERER_GEOMETRY;
using namespace D_CORE;

namespace Darius::ResourceManager
{
	class DResourceManager;

	class BatchResource : public MeshResource
	{
		D_CH_RESOURCE_BODY(BatchResource, ResourceType::Batch)

	public:

		INLINE virtual ResourceType		GetType() const override { return ResourceType::Batch; }

		virtual void					Create(std::wstring name, MeshData<VertexType>& data) override;
		
	private:
		friend class DResourceManager;

		BatchResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, id, isDefault) {}

	};

}