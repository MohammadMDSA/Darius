#include "Renderer/pch.hpp"
#include "SkeletalMeshRendererComponent.hpp"

#include "Renderer/RendererManager.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Graphics/GraphicsUtils/Shader/Shaders.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>
#include <ResourceManager/ResourceManager.hpp>

#define FBXSDK_SHARED
#include <fbxsdk.h>

#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "SkeletalMeshRendererComponent.sgenerated.hpp"

using namespace D_CORE;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_RENDERER;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;
using namespace D_SERIALIZATION;

namespace Darius::Renderer
{
	D_H_COMP_DEF(SkeletalMeshRendererComponent);

	// Internal
	ComputePSO				AsyncAnimationCS(L"Skeletal Mesh Async Animation CS");

	void InitialziePSOs()
	{
		if (AsyncAnimationCS.GetPipelineStateObject())
			return;

		AsyncAnimationCS.SetRootSignature(D_GRAPHICS::CommonRS);
		auto shader = D_GRAPHICS::GetShaderByName<D_GRAPHICS_SHADERS::ComputeShader>("SkeletalMeshAnimationDeformationCS");
		AsyncAnimationCS.SetComputeShader(shader);
		AsyncAnimationCS.Finalize();
	}

	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent() :
		MeshRendererComponentBase(),
		mSkeletonRoot(nullptr),
		mMesh(),
		mFitBounds(false)
	{
		mComponentPsoFlags |= RenderItem::HasSkin;

		if (AsyncAnimationCS.GetPipelineStateObject() == nullptr)
			InitialziePSOs();
	}

	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent(D_CORE::Uuid const& uuid) :
		MeshRendererComponentBase(uuid),
		mSkeletonRoot(nullptr),
		mMesh(),
		mFitBounds(false)
	{
		mComponentPsoFlags |= RenderItem::HasSkin;

		if (AsyncAnimationCS.GetPipelineStateObject() == nullptr)
			InitialziePSOs();
	}

	bool SkeletalMeshRendererComponent::AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext)
	{

		if (mMesh->IsDirtyGPU())
			return false;

		bool any = false;
		auto result = RenderItem();
		const Mesh* mesh = mMesh.Get()->GetMeshData();
		auto const& meshMaterials = mMesh->GetMaterials();
		result.Mesh = mesh;
		result.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		result.MeshVsCBV = GetConstantsAddress();
		result.mJointData = mJoints.data();
		result.mNumJoints = (UINT)mJoints.size();

#if _D_EDITOR
		if (riContext.IsEditor)
		{
			if (GetGameObject() == riContext.SelectedGameObject)
			{
				result.StencilEnable = true;
				result.CustomDepth = true;
				result.StencilValue = riContext.StencilOverride;
			}
			else
			{
				result.StencilEnable = false;
				result.CustomDepth = false;
				result.StencilValue = 0;
			}
		}
		else
#endif
		{
			result.StencilEnable = IsStencilWriteEnable();
			result.StencilValue = GetStencilValue();
			result.CustomDepth = IsCustomDepthEnable();
		}

		UINT draws = (UINT)mesh->mDraw.size();
		if (draws != (UINT)mMaterials.size())
			OnMeshChanged();

		for (UINT i = 0; i < draws; i++)
		{
			auto const& draw = mesh->mDraw[i];

			ResourceRef<MaterialResource> material = mMaterials[i];

			if (!mMaterials[i].IsValid())
				material = meshMaterials[i];

			if (!material.IsValid() || material->IsDirtyGPU())
				continue;

			result.PsoType = GetPsoIndex(i, material.Get());
			result.DepthPsoIndex = mMaterialPsoData[i].DepthPsoIndex;
			result.Material.MaterialCBV = material->GetConstantsGpuAddress();
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

	bool SkeletalMeshRendererComponent::CanRenderForPicker() const
	{
		if(mMesh.IsNull() || mMesh->IsDirtyGPU())
			return false;

		return true;
	}

	RenderItem SkeletalMeshRendererComponent::GetPickerRenderItem() const
	{
		static RenderItem ri;
		struct PsoInitializer
		{
			PsoInitializer()
			{
				uint32_t flags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::HasSkin;

				D_RENDERER_RAST::PsoConfig config;
				config.PsoFlags = flags;

				config.PSIndex = D_GRAPHICS::GetShaderIndex("EditorPickerPS");
				config.VSIndex = D_GRAPHICS::GetShaderIndex("SkinnedVS");
				config.RenderRargetFormats = {DXGI_FORMAT_R32G32_UINT};

				uint32_t psoIndex = D_RENDERER_RAST::GetPso(config);
				config.PsoFlags |= RenderItem::DepthOnly;
				config.PSIndex = 0;
				uint32_t depthPsoIndex = D_RENDERER_RAST::GetPso(config);

				ri.PsoType = psoIndex;
				ri.DepthPsoIndex = depthPsoIndex;
				ri.Material.MaterialSRV = {0};
				ri.Material.SamplersSRV = {0};
				ri.PsoFlags = flags;
				ri.BaseVertexLocation = 0u;
				ri.StartIndexLocation = 0u;
			}
		};

		static PsoInitializer initializer;

		const Mesh* mesh = mMesh.Get()->GetMeshData();
		ri.MeshVsCBV = GetConstantsAddress();
		ri.Material.MaterialCBV = GetPickerDrawPsConstant();
		ri.Mesh = mesh;
		ri.mJointData = mJoints.data();
		ri.mNumJoints = (UINT)mJoints.size();
		ri.IndexCount = mesh->mNumTotalIndices;

		return ri;
	}

	bool SkeletalMeshRendererComponent::DrawDetails(float params[])
	{
		auto valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Mesh 
		{
			D_H_DETAILS_DRAW_PROPERTY("Mesh");
			D_H_RESOURCE_SELECTION_DRAW(SkeletalMeshResource, mMesh, "Select Mesh", SetMesh);
		}

		// Fit Bounds
		{
			D_H_DETAILS_DRAW_PROPERTY("Fit Bounds");
			auto value = IsFitBounds();
			if(ImGui::Checkbox("##FitBounds", &value))
			{
				SetFitBounds(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		valueChanged |= MeshRendererComponentBase::DrawDetails(params);

		return valueChanged;

	}

	void SkeletalMeshRendererComponent::OnGizmo() const
	{
#if 0 // Rendering debug joints and joint bounds
		auto transform = GetTransform();
		auto world = transform->GetWorld();
		for (auto const& joint : mJointLocalPoses)
		{
			Vector3 loc = Vector3(world * joint);
			D_DEBUG_DRAW::DrawSphere(loc, 0.01f, 0., Color::Red);
		}

		for(auto const& bound : mJointBounds)
		{
			auto worldSp = AffineTransform(world) * bound;
			D_DEBUG_DRAW::DrawCube(worldSp.GetCenter(), worldSp.GetOrientation(), 2 * worldSp.GetExtents());
		}
#endif
	}
#endif

	void SkeletalMeshRendererComponent::SetMesh(SkeletalMeshResource* mesh)
	{

		if (!CanChange())
			return;

		if (mMesh == mesh)
			return;

		mMesh = mesh;

		auto meshValid = mMesh.IsValid();

		if (meshValid && mMesh->IsLoaded())
			LoadMeshData();
		else if (meshValid)
		{
			D_RESOURCE_LOADER::LoadResourceAsync(mesh, [&](auto resource)
				{
					LoadMeshData();
				}, true);

		}
		else // To destroy deformed mesh buffer and 
		{
			LoadMeshData();
		}

		mChangeSignal(this);
	}

	void SkeletalMeshRendererComponent::SetFitBounds(bool fitBounds)
	{
		if(mFitBounds == fitBounds)
			return;

		mFitBounds = fitBounds;

		mChangeSignal(this);
	}

	void SkeletalMeshRendererComponent::LoadMeshData()
	{
		OnMeshChanged();
		mJoints.clear();
		mSkeleton.clear();
		mJointLocalPoses.clear();
		mJointBounds.clear();
		mSkeletonRoot = nullptr;
		mBounds = D_MATH_BOUNDS::Aabb(GetTransform()->GetPosition());
		if (mMesh.IsValid())
		{
			// Copying skeleton to component
			mJoints.resize(mMesh->GetJointCount());
			mSkeleton.reserve(mMesh->GetJointCount());

#if _D_EDITOR
			mJointLocalPoses.resize(mMesh->GetJointCount());
			mJointBounds.resize(mMesh->GetJointCount());
#endif

			auto refSkeleton = mMesh->GetSkeleton();
			std::ranges::copy(refSkeleton.begin(), refSkeleton.end(), std::back_inserter(mSkeleton));

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

			// Create deformed mesh buffer
			if (D_RENDERER::GetActiveRendererType() == D_RENDERER::RendererType::RayTracing)
			{
				auto const* meshData = mMesh->GetMeshData();
				mDeformedMesh = D_RENDERER_GEOMETRY::Mesh(*meshData, L"Skeletal Mesh Deformed Mesh", false, true);
				mDeformedMesh.VertexDataGpu.Create(L"Skeletal Mesh Deformed Mesh Vertices", mDeformedMesh.mNumTotalVertices, sizeof(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture));

				auto& context = D_GRAPHICS::ComputeContext::Begin(L"Copy Index Buffer to for deformed mesh indices");
				mDeformedMesh.FillIndices(const_cast<D_RENDERER_GEOMETRY::Mesh&>(*meshData), context);

				context.Finish();
			}

			// Joints buffers
			if (mJoints.size() > 0)
			{
				mJointsBufferGpu.Create(L"Skeletal Mesh Joint Buffer", (UINT)mJoints.size(), sizeof(D_RENDERER::Joint));
				mJointsBufferUpload.Create(L"Skeletal Mesh Joint Buffer Upload", (UINT)(mJoints.size() * sizeof(D_RENDERER::Joint)), D_GRAPHICS_DEVICE::gNumFrameResources);
			}
		}
		else
		{
			mDeformedMesh.Destroy();
			mJointsBufferGpu.Destroy();
			mJointsBufferUpload.Destroy();
		}
	}

	void SkeletalMeshRendererComponent::JointUpdateRecursion(Matrix4 const& parent, Mesh::SkeletonJoint& skeletonJoint)
	{

		if (skeletonJoint.StaleMatrix)
		{
			skeletonJoint.StaleMatrix = false;

			fbxsdk::FbxAMatrix mat;
			mat.SetTRS({ 0, 0, 0 }, { skeletonJoint.Rotation.GetX(), skeletonJoint.Rotation.GetY(), skeletonJoint.Rotation.GetZ(), 1.f }, { 1, 1, 1 });
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

		OrientedBox jointBounds = AffineTransform(withOffset) * OrientedBox(skeletonJoint.Aabb);
#if _D_EDITOR
		mJointLocalPoses[skeletonJoint.MatrixIdx] = Vector3(xform.GetW());
		mJointBounds[skeletonJoint.MatrixIdx] = jointBounds;
#endif
		
		Scalar scaleXSqr = LengthSquare((Vector3)xform.GetX());
		Scalar scaleYSqr = LengthSquare((Vector3)xform.GetY());
		Scalar scaleZSqr = LengthSquare((Vector3)xform.GetZ());

		Aabb jointAabb;
		if(mFitBounds)
			jointAabb = jointBounds.GetAabb();
		else
			jointAabb = jointBounds.GetAabbFast();

		mBounds = mBounds.Union(jointAabb);

		for (auto childJoint : skeletonJoint.Children)
		{
			JointUpdateRecursion(xform, *childJoint);
		}
	}

	void SkeletalMeshRendererComponent::OnDestroy()
	{
		mDeformedMesh.Destroy();

		Super::OnDestroy();
	}

	void SkeletalMeshRendererComponent::Update(float dt)
	{
		if (!IsDirty() || !IsActive())
			return;

		if (!mMesh.IsValid())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Skeletal Mesh Update");

		auto frameResourceIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();

		// Updating mesh constants
		// Mapping upload buffer
		MeshConstants* cb = (MeshConstants*)mMeshConstantsCPU.MapInstance(frameResourceIndex);

		auto transform = GetTransform();
		Matrix4 world = transform->GetWorld();
		cb->World = world;
		cb->WorldIT = InverseTranspose(world.Get3x3());
		cb->Lod = mLoD;

		// Updating joints matrices for gpu
		if (mSkeletonRoot)
		{
			mBounds = Aabb(transform->GetPosition());
			JointUpdateRecursion(Matrix4::Identity, *mSkeletonRoot);
		}

		// Copy to gpu
		{
			mMeshConstantsCPU.Unmap();

			// Uploading
			context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
			context.CopyBufferRegion(mMeshConstantsGPU, 0, mMeshConstantsCPU, frameResourceIndex * mMeshConstantsCPU.GetBufferSize(), mMeshConstantsCPU.GetBufferSize());
			context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
		}

		// Uploading joints
		if(mJoints.size() > 0)
		{
			D_PROFILING::ScopedTimer _prof2(L"Joints Upload", context);

			// Storing to upload buffer
			D_RENDERER::Joint* mapped = (D_RENDERER::Joint*)mJointsBufferUpload.MapInstance(frameResourceIndex);
			std::memcpy(mapped, mJoints.data(), mJointsBufferUpload.GetBufferSize());
			mJointsBufferUpload.Unmap();

			context.UploadToBuffer(mJointsBufferGpu, mJointsBufferUpload);
		}

		bool isRayTracing = D_RENDERER::GetActiveRendererType() == D_RENDERER::RendererType::RayTracing;

		auto& prototypeVertBuffer = const_cast<D_GRAPHICS_BUFFERS::StructuredBuffer&>(mMesh->GetMeshData()->VertexDataGpu);

		if (isRayTracing)
		{
			context.TransitionResource(prototypeVertBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		uint64_t bufferPrep = context.Finish();


		// Animating the deformed mesh
		if (D_RENDERER::GetActiveRendererType() == D_RENDERER::RendererType::RayTracing)
		{
			
			// Wait to have joint buffers ready
			D_GRAPHICS::GetCommandManager()->GetComputeQueue().StallForFence(bufferPrep);

			auto& compute = D_GRAPHICS::ComputeContext::Begin(L"Async Skeletal Animation Mesh Deformation", true);

			compute.TransitionResource(mDeformedMesh.VertexDataGpu, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			compute.TransitionResource(mJointsBufferGpu, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			compute.SetPipelineState(AsyncAnimationCS);
			compute.SetRootSignature(D_GRAPHICS::CommonRS);
			compute.SetDynamicDescriptor(1, 0, prototypeVertBuffer.GetSRV());
			compute.SetDynamicDescriptor(1, 1, mJointsBufferGpu.GetSRV());
			compute.SetDynamicDescriptor(2, 0, mDeformedMesh.VertexDataGpu.GetUAV());
			compute.Dispatch2D(prototypeVertBuffer.GetElementCount(), 1u, 256u, 1u);
			compute.Finish();
		}

		Super::Update(dt);

		SetClean();
	}

	D_MATH_BOUNDS::Aabb SkeletalMeshRendererComponent::GetAabb() const
	{
		auto transform = GetTransform();

		if(!mMesh.IsValid())
			return D_MATH_BOUNDS::Aabb(transform->GetPosition());

		auto affiteTransform = AffineTransform(transform->GetWorld());

		return mBounds.CalculateTransformed(affiteTransform);
	}

	void SkeletalMeshRendererComponent::OnDeserialized()
	{
		Super::OnDeserialized();

		for (UINT i = 0; i < mMaterialPsoData.size(); i++)
			mMaterialPsoData[i].PsoIndexDirty = true;

		LoadMeshData();
	}

	void SkeletalMeshRendererComponent::GetOverriddenMaterials(D_CONTAINERS::DVector<MaterialResource*>& out) const
	{
		if (!mMesh.IsValid() || !mMesh->IsLoaded())
		{
			out.resize(0);
			return;
		}

		out.resize(mMaterials.size());
		
		for (int i = 0; i < out.size(); i++)
		{
			auto const& overrideMat = mMaterials[i];
			if (overrideMat.IsValid())
			{
				out[i] = overrideMat.Get();
				continue;
			}
			out[i] = mMesh->GetMaterial(i);
		}
	}
}
