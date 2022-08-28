#pragma once

#include "EntityComponentSystem/Entity.hpp"

#include <Core/Ref.hpp>
#include <Core/Uuid.hpp>
#include <Core/Serialization/Json.hpp>
#include <Math/VectorMath.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/FrameResource.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/CommandContext.hpp>
#include <Renderer/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <ResourceManager/ResourceTypes/MeshResource.hpp>
#include <ResourceManager/ResourceTypes/MaterialResource.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

using namespace D_MATH;
using namespace D_RENDERER_FRAME_RESOUCE;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_RESOURCE;
using namespace D_CORE;

namespace Darius::Scene::ECS::Components
{
	class ComponentBase;
}

namespace Darius::Scene
{
	class SceneManager;
	class GameObject;

	void to_json(D_SERIALIZATION::Json& j, const GameObject& value);
	void from_json(const D_SERIALIZATION::Json& j, GameObject& value);

	class GameObject
	{
	public:
		enum class Type
		{
			Static,
			Movable
		};

	public:
		~GameObject();

		RenderItem							GetRenderItem();
		INLINE bool							CanRender() { return mActive && mMeshResource.IsValid(); }
		INLINE const BoundingSphere&		GetBounds() const { return mMeshResource.Get()->GetData()->mBoundSp; }

		void								SetTransform(Transform const& trans);
		Transform const*					GetTransform() const;

		template<class T>
		INLINE T*							GetComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);
			return mEntity.get_ref<T>().get();
		}

#ifdef _D_EDITOR
		bool								DrawDetails(float params[]);
#endif // _EDITOR

		INLINE operator CountedOwner const() {
			return CountedOwner{ WSTR_STR(mName), "GameObject", this, 0 };
		}

		void								SetMesh(ResourceHandle handle);
		void								SetMaterial(ResourceHandle handle);

		D_CH_RW_FIELD(bool, Active);
		D_CH_RW_FIELD(std::string, Name);
		D_CH_RW_FIELD(Type, Type);
		D_CH_R_FIELD_CONST(Uuid, Uuid);
		D_CH_R_FIELD(D_ECS::Entity, Entity);

	private:
		friend class D_SCENE::SceneManager;
		friend void							to_json(D_SERIALIZATION::Json& j, const GameObject& value);
		friend void							from_json(const D_SERIALIZATION::Json& j, GameObject& value);

		GameObject(Uuid uuid, D_ECS::Entity entity);

		void								Update(D_GRAPHICS::GraphicsContext& context, float deltaTime);

		Ref<MeshResource>					mMeshResource;
		Ref<MaterialResource>				mMaterialResouce;

		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMeshConstantsGPU;

	};

	D_H_SERIALIZE_ENUM(GameObject::Type, {
		{ GameObject::Type::Static, "Static" },
		{ GameObject::Type::Movable, "Movable" }
		});

	void to_json(D_SERIALIZATION::Json& j, const GameObject& value);

	void from_json(const D_SERIALIZATION::Json& j, GameObject& value);
}
