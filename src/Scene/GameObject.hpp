#pragma once
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/FrameResource.hpp>
#include <Math/VectorMath.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

using namespace D_MATH;
using namespace D_RENDERER_FRAME_RESOUCE;

namespace Darius::Scene
{
	class GameObject
	{
	public:
		GameObject();
		~GameObject();
		RenderItem					GetRenderItem();

		inline std::string const	GetName();
		void						SetMesh(std::shared_ptr<D_RENDERER_GEOMETRY::Mesh> mesh);

		Transform										mTransform;

	private:
		bool											mActive;
		std::string										mName;
		std::shared_ptr<D_RENDERER_GEOMETRY::Mesh>		mMesh;
	};

	inline std::string const GameObject::GetName()
	{
		return "mName";
	}
}
