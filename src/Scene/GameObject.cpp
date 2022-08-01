#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "GameObject.hpp"

namespace Darius::Scene
{

	GameObject::GameObject() :
		mActive(false),
		mName("GameObject"),
		mMesh(nullptr)
	{
		mTransform = Transform::MakeIdentity();
	}

	GameObject::~GameObject()
	{
	}
	void GameObject::SetMesh(std::shared_ptr<D_RENDERER_GEOMETRY::Mesh> mesh)
	{
		mMesh = mesh;
	}
	RenderItem GameObject::GetRenderItem()
	{
		auto result = RenderItem();
		result.BaseVertexLocation = mMesh->mDraw[0].mBaseVertexLocation;
		result.IndexCount = mMesh->mDraw[0].mIndexCount;
		result.StartIndexLocation = mMesh->mDraw[0].mStartIndexLocation;
		result.Mesh = mMesh.get();
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.World = mTransform;
		return result;
	}
}
