#include "Renderer/pch.hpp"
#include "SkeletalMeshRendererComponent.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Utils/DragDropPayload.hpp>

#include <imgui.h>
#define FBXSDK_SHARED
#include <fbxsdk.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

#include "SkeletalMeshRendererComponent.sgenerated.hpp"

using namespace D_CORE;
using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_SERIALIZATION;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;
using namespace D_RENDERER_FRAME_RESOURCE;

namespace Darius::Graphics
{
	D_H_COMP_DEF(SkeletalMeshRendererComponent);


	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent() :
		MeshRendererComponentBase(),
		mSkeletonRoot(nullptr),
		mMesh(GetAsCountedOwner())
	{
		mComponentPsoFlags |= RenderItem::HasSkin;
	}

	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent(D_CORE::Uuid uuid) :
		MeshRendererComponentBase(uuid),
		mSkeletonRoot(nullptr),
		mMesh(GetAsCountedOwner())
	{
		mComponentPsoFlags |= RenderItem::HasSkin;
	}

	bool SkeletalMeshRendererComponent::AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction)
	{
		bool any = false;
		auto result = RenderItem();
		const Mesh* mesh = mMesh.Get()->GetMeshData();
		result.Mesh = mesh;
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.MeshCBV = GetConstantsAddress();
		result.mJointData = mJoints.data();
		result.mNumJoints = (UINT)mJoints.size();

		for (UINT i = 0; i < mesh->mDraw.size(); i++)
		{
			auto const& draw = mesh->mDraw[i];
			auto const& material = mMaterials[i];

			if (!material.IsValid() || material->IsDirtyGPU())
				continue;

			result.PsoType = GetPsoIndex(i);
			result.Material.MaterialCBV = *material.Get();
			result.Material.MaterialSRV = material->GetTexturesHandle();
			result.Material.SamplersSRV = material->GetSamplersHandle();
			result.PsoFlags = mComponentPsoFlags | material->GetPsoFlags();
			result.BaseVertexLocation = mesh->mDraw[i].BaseVertexLocation;
			result.StartIndexLocation = mesh->mDraw[i].StartIndexLocation;
			result.IndexCount = mesh->mDraw[i].IndexCount;

			appendFunction(result);

			any = true;
		}

		return any;
	}

#ifdef _D_EDITOR
	bool SkeletalMeshRendererComponent::DrawDetails(float params[])
	{
		auto valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		valueChanged |= MeshRendererComponentBase::DrawDetails(params);

		// Mesh selection
		D_H_DETAILS_DRAW_PROPERTY("Mesh");
		D_H_RESOURCE_SELECTION_DRAW(SkeletalMeshResource, mMesh, "Select Mesh", SetMesh);

		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
			mChangeSignal();

		return valueChanged;

	}
#endif

	void SkeletalMeshRendererComponent::_SetMesh(ResourceHandle handle)
	{
		mMesh = D_RESOURCE::GetResource<SkeletalMeshResource>(handle, *this);
		LoadMeshData();
	}

	void SkeletalMeshRendererComponent::LoadMeshData()
	{
		OnMeshChanged();
		mJoints.clear();
		mSkeleton.clear();
		mSkeletonRoot = nullptr;
		if (mMesh.IsValid())
		{
			// Copying skeleton to component
			mJoints.resize(mMesh->GetJointCount());
			for (auto const& skeletonNode : mMesh->GetSkeleton())
			{
				mSkeleton.push_back(skeletonNode);
			}

			// Attaching new children refrences and finding the root
			for (auto& skeletonNode : mSkeleton)
			{
				// Storing children indices
				D_CONTAINERS::DSet<int> childrendIndices;
				for (auto const& child : skeletonNode.Children)
					childrendIndices.insert(child->MatrixIdx);

				skeletonNode.Children.clear();

				for (auto childIndex : childrendIndices)
					skeletonNode.Children.push_back(&mSkeleton[childIndex]);

				if (skeletonNode.SkeletonRoot)
				{
					mSkeletonRoot = &skeletonNode;
				}
			}
		}
	}

	void SkeletalMeshRendererComponent::JointUpdateRecursion(Matrix4 const& parent, Mesh::SkeletonJoint& skeletonJoint)
	{

		if (skeletonJoint.StaleMatrix)
		{
			skeletonJoint.StaleMatrix = false;

			fbxsdk::FbxAMatrix mat;
			mat.SetTRS({ 0, 0, 0 }, { skeletonJoint.Rotation.x, skeletonJoint.Rotation.y, skeletonJoint.Rotation.z, 1.f }, { 1, 1, 1 });
			auto quat = mat.GetQ();


			skeletonJoint.Xform.Set3x3(Matrix3(Quaternion((float)quat.mData[0], (float)quat.mData[1], (float)quat.mData[2], (float)quat.mData[3])) * Matrix3::MakeScale(skeletonJoint.Scale));
		}

		auto xform = skeletonJoint.Xform;
		if (!skeletonJoint.SkeletonRoot)
			xform = parent * xform;

		auto& joint = mJoints[skeletonJoint.MatrixIdx];
		auto withOffset = xform * skeletonJoint.IBM;
		joint.mWorld = withOffset;
		joint.mWorldIT = InverseTranspose(withOffset.Get3x3());


		Scalar scaleXSqr = LengthSquare((Vector3)xform.GetX());
		Scalar scaleYSqr = LengthSquare((Vector3)xform.GetY());
		Scalar scaleZSqr = LengthSquare((Vector3)xform.GetZ());
		Scalar sphereScale = Sqrt(Max(Max(scaleXSqr, scaleYSqr), scaleZSqr));
		mBounds = mBounds.Union(BoundingSphere((Vector3)xform.GetW(), sphereScale));

		for (auto childJoint : skeletonJoint.Children)
		{
			JointUpdateRecursion(xform, *childJoint);
		}
	}

	void SkeletalMeshRendererComponent::Update(float dt)
	{
		if (!IsActive())
			return;

		if (!mMesh.IsValid())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];
		MeshConstants* cb = (MeshConstants*)currentUploadBuff.Map();


		Matrix4 world = Matrix4(GetTransform().GetWorld());
		cb->mWorld = world;
		cb->mWorldIT = InverseTranspose(world.Get3x3());

		// Updating joints matrices on gpu
		if (mSkeletonRoot)
		{

			JointUpdateRecursion(Matrix4(), *mSkeletonRoot);
		}

		// Copy to gpu
		{
			currentUploadBuff.Unmap();

			// Uploading
			context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
			context.GetCommandList()->CopyBufferRegion(mMeshConstantsGPU.GetResource(), 0, currentUploadBuff.GetResource(), 0, currentUploadBuff.GetBufferSize());
			context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		context.Finish();

	}

	void SkeletalMeshRendererComponent::OnDeserialized()
	{
		for(UINT i = 0; i < mMaterialPsoData.size(); i++)
			mMaterialPsoData[i].PsoIndexDirty = true;

		LoadMeshData();
	}

}
