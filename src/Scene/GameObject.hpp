#pragma once

#include <Core/Ref.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/FrameResource.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Math/VectorMath.hpp>
#include <ResourceManager/MeshResource.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

using namespace D_MATH;
using namespace D_RENDERER_FRAME_RESOUCE;
using namespace D_RESOURCE;
using namespace D_CORE;

namespace Darius::Scene
{
	class GameObject
	{
	public:
		GameObject();
		~GameObject();
		RenderItem					GetRenderItem();

		inline std::string const	GetName() { return mName; }

#ifdef _D_EDITOR
		bool						DrawInspector(float params[]);
#endif // _EDITOR


		Transform										mTransform;

		Ref<MeshResource>								mMeshResource;

		INLINE operator CountedOwner const() {
			return CountedOwner { WSTR_STR(mName), "GameObject", this, 0};
		}
	private:
		bool											mActive;
		std::string										mName;
	};
}
