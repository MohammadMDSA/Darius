#pragma once

#include <Core/Ref.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/FrameResource.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/CommandContext.hpp>
#include <Renderer/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Math/VectorMath.hpp>
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

namespace Darius::Scene
{
	class SceneManager;

	class GameObject
	{
	public:
		enum class Type
		{
			Static,
			Movable
		};

	public:
		GameObject();
		~GameObject();

		RenderItem					GetRenderItem();
		INLINE bool					CanRender() { return mActive && mMeshResource.IsValid(); }
		
		INLINE const BoundingSphere& GetBounds() const { return mMeshResource.Get()->GetData()->mBoundSp; }

#ifdef _D_EDITOR
		bool						DrawDetails(float params[]);
#endif // _EDITOR


		Transform										mTransform;


		INLINE operator CountedOwner const() {
			return CountedOwner { WSTR_STR(mName), "GameObject", this, 0};
		}

		void						SetMesh(ResourceHandle handle);
		void						SetMaterial(ResourceHandle handle);

		D_CH_RW_FIELD(bool, Active);
		D_CH_RW_FIELD(std::string, Name);
		D_CH_RW_FIELD(Type, Type);
	private:
		friend class D_SCENE::SceneManager;

		void						Update(D_GRAPHICS::GraphicsContext& context, float deltaTime);

		Ref<MeshResource>					mMeshResource;
		Ref<MaterialResource>				mMaterialResouce;

		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMeshConstantsGPU;

	};
}
