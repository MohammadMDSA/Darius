#pragma once
#include <Renderer/Geometry/Mesh.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		void			SetMesh(std::shared_ptr<D_RENDERER_GEOMETRY::Mesh> mesh);
	private:
		bool											mActive;
		std::string										mName;
		std::shared_ptr<D_RENDERER_GEOMETRY::Mesh>		mMesh;
	};

}
