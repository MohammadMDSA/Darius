#pragma once

#include "Resource.hpp"

#include <Renderer/Geometry/MeshData.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/GraphicsUtils/VertexTypes.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_RENDERER_GEOMETRY;

namespace Darius::ResourceManager
{
	class MeshResource : public Resource
	{
		using VertexType = D_RENDERER_VERTEX::VertexPositionNormalTangentTexture;

		D_CH_RESOUCE_BODY(MeshResource, ResourceType::Mesh)

	public:
		MeshResource(DResourceId id) : Resource(id) {}
		INLINE Mesh*					Get() { return &mMesh; }
		INLINE const Mesh*				Get() const { return &mMesh; }

		INLINE virtual ResourceType		GetType() const override { return ResourceType::Mesh; }

		void							Create(std::wstring name, MeshData<VertexType>& data);
		void							Create(std::wstring path);

		INLINE operator Mesh* const() { return &mMesh; }

	private:
		Mesh							mMesh;
	};
}