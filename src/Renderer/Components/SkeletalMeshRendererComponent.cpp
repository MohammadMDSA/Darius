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


using namespace D_SERIALIZATION;
using namespace D_RESOURCE;
using namespace D_RENDERER_FRAME_RESOURCE;

namespace Darius::Graphics
{
	D_H_COMP_DEF(SkeletalMeshRendererComponent);


	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent() :
		D_ECS_COMP::ComponentBase(),
		mComponentPsoFlags(RenderItem::HasSkin),
		mCachedMaterialPsoFlags(0),
		mPsoIndex(0),
		mPsoIndexDirty(true),
		mSkeletonRoot(nullptr)
	{
	}

	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent(D_CORE::Uuid uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mComponentPsoFlags(RenderItem::HasSkin),
		mCachedMaterialPsoFlags(0),
		mPsoIndex(0),
		mPsoIndexDirty(true),
		mSkeletonRoot(nullptr)
	{
	}

	void SkeletalMeshRendererComponent::Awake()
	{

		// Initializing Mesh Constants buffers
		CreateGPUBuffers();

		if (!mMaterialResource.IsValid())
			_SetMaterial(D_GRAPHICS::GetDefaultGraphicsResource(DefaultResource::Material));
	}

	RenderItem SkeletalMeshRendererComponent::GetRenderItem()
	{
		auto result = RenderItem();
		const Mesh* mesh = mMeshResource.Get()->GetMeshData();
		result.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		result.IndexCount = mesh->mNumTotalIndices;
		result.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		result.Mesh = mesh;
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.MeshCBV = GetConstantsAddress();
		result.Material.MaterialCBV = *mMaterialResource.Get();
		result.Material.MaterialSRV = mMaterialResource->GetTexturesHandle();
		result.mJointData = mJoints.data();
		result.mNumJoints = mJoints.size();
		result.PsoType = GetPsoIndex();
		result.PsoFlags = mComponentPsoFlags | mMaterialResource->GetPsoFlags();
		return result;
	}

#ifdef _D_EDITOR
	bool SkeletalMeshRendererComponent::DrawDetails(float params[])
	{
		auto valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Mesh selection
		D_H_DETAILS_DRAW_PROPERTY("Mesh");
		D_H_RESOURCE_SELECTION_DRAW(SkeletalMeshResource, mMeshResource, "Select Mesh", SetMesh);

		// Material selection
		D_H_DETAILS_DRAW_PROPERTY("Material");
		D_H_RESOURCE_SELECTION_DRAW(MaterialResource, mMaterialResource, "Select Material", SetMaterial);

		// Casting shadow
		D_H_DETAILS_DRAW_PROPERTY("Casts Shadow");
		valueChanged |= ImGui::Checkbox("##CastsShadow", &mCastsShadow);

		D_H_DETAILS_DRAW_END_TABLE();

		if (valueChanged)
			mChangeSignal();

		return valueChanged;

	}
#endif


	void SkeletalMeshRendererComponent::SetMesh(ResourceHandle handle)
	{
		mChangeSignal();
		_SetMesh(handle);
	}

	void SkeletalMeshRendererComponent::SetMaterial(ResourceHandle handle)
	{
		mChangeSignal();
		_SetMaterial(handle);
	}

	void SkeletalMeshRendererComponent::_SetMesh(ResourceHandle handle)
	{
		mMeshResource = D_RESOURCE::GetResource<SkeletalMeshResource>(handle, *this);

		mJoints.clear();
		mSkeleton.clear();
		mSkeletonRoot = nullptr;
		if (mMeshResource.IsValid())
		{
			// Copying skeleton to component
			mJoints.resize(mMeshResource->GetJointCount());
			for (auto const& skeletonNode : mMeshResource->GetSkeleton())
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

	void SkeletalMeshRendererComponent::_SetMaterial(ResourceHandle handle)
	{
		mPsoIndexDirty = true;
		mMaterialResource = D_RESOURCE::GetResource<MaterialResource>(handle, *this);
	}

	void SkeletalMeshRendererComponent::Serialize(Json& j) const
	{
		if (mMaterialResource.IsValid())
			D_CORE::to_json(j["Material"], mMaterialResource.Get()->GetUuid());
		if (mMeshResource.IsValid())
			D_CORE::to_json(j["Mesh"], mMeshResource.Get()->GetUuid());
	}

	void SkeletalMeshRendererComponent::Deserialize(Json const& j)
	{
		auto go = GetGameObject();

		// Loading material
		if (j.contains("Material"))
		{
			Uuid materialUuid;
			D_CORE::from_json(j["Material"], materialUuid);
			_SetMaterial(*D_RESOURCE::GetResource<MaterialResource>(materialUuid, *go));
		}

		if (j.contains("Mesh"))
		{
			// Loading mesh
			Uuid meshUuid;
			D_CORE::from_json(j["Mesh"], meshUuid);
			_SetMesh(*D_RESOURCE::GetResource<SkeletalMeshResource>(meshUuid, *go));
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


			skeletonJoint.Xform.Set3x3(Matrix3(Quaternion(quat.mData[0], quat.mData[1], quat.mData[2], quat.mData[3])) * Matrix3::MakeScale(skeletonJoint.Scale));
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

		// We won't update constant buffer for static objects
		if (GetGameObject()->GetType() == D_SCENE::GameObject::Type::Static)
			return;

		if (!mMeshResource.IsValid())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
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

	void SkeletalMeshRendererComponent::OnDestroy()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Destroy();
		}
		mMeshConstantsGPU.Destroy();
	}

	void SkeletalMeshRendererComponent::CreateGPUBuffers()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));
	}
}