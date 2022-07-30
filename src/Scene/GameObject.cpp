#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "GameObject.hpp"

namespace Darius::Scene
{

	GameObject::GameObject()
	{
	}

	GameObject::~GameObject()
	{
	}
	void GameObject::SetMesh(std::shared_ptr<D_RENDERER_GEOMETRY::Mesh> mesh)
	{
		mMesh = mesh;
	}
}
