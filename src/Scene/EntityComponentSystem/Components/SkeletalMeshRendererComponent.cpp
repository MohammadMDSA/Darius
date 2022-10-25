#include "Scene/pch.hpp"
#include "SkeletalMeshRendererComponent.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"

#include <Debug/DebugDraw.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include <imgui.h>

using namespace D_SERIALIZATION;
using namespace D_RESOURCE;
using namespace D_RENDERER_FRAME_RESOUCE;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(SkeletalMeshRendererComponent);


	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent() :
		ComponentBase(),
		mPsoFlags(0)
	{
	}

	SkeletalMeshRendererComponent::SkeletalMeshRendererComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mPsoFlags(0)
	{
	}

	void SkeletalMeshRendererComponent::Start()
	{

		// Initializing Mesh Constants buffers
		CreateGPUBuffers();

		if (!mMaterialResource.IsValid())
			_SetMaterial(D_RESOURCE::GetDefaultResource(DefaultResource::Material));
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
		result.PsoType = D_RENDERER::SkinnedOpaquePso;
		result.PsoFlags = mPsoFlags | mMaterialResource->GetPsoFlags();
		return result;
	}

#ifdef _D_EDITOR
	bool SkeletalMeshRendererComponent::DrawDetails(float params[])
	{
		auto changeValue = false;

		if (ImGui::BeginTable("mesh editor", 2, ImGuiTableFlags_BordersInnerV))
		{
			// Shader type
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Mesh");
			ImGui::TableSetColumnIndex(1);
			// Mesh selection
			{
				MeshResource* currentMesh = mMeshResource.Get();

				if (ImGui::Button("Select"))
				{
					ImGui::OpenPopup("Select Res");
				}

				if (ImGui::BeginPopup("Select Res"))
				{
					auto meshes = D_RESOURCE::GetResourcePreviews(SkeletalMeshResource::GetResourceType());
					int idx = 0;
					for (auto prev : meshes)
					{
						bool selected = currentMesh && prev.Handle.Id == currentMesh->GetId() && prev.Handle.Type == currentMesh->GetType();

						auto Name = STR_WSTR(prev.Name);
						ImGui::PushID((Name + std::to_string(idx)).c_str());
						if (ImGui::Selectable(Name.c_str(), &selected))
						{
							SetMesh(prev.Handle);
							changeValue = true;
						}
						ImGui::PopID();

						idx++;
					}

					ImGui::EndPopup();
				}
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Material");
			ImGui::TableSetColumnIndex(1);
			// Material selection
			{
				MaterialResource* currentMaterial = mMaterialResource.Get();

				if (ImGui::Button("Select"))
				{
					ImGui::OpenPopup("Select Mat");
				}

				if (ImGui::BeginPopup("Select Mat"))
				{
					auto meshes = D_RESOURCE::GetResourcePreviews(MaterialResource::GetResourceType());
					int idx = 0;
					for (auto prev : meshes)
					{
						bool selected = currentMaterial && prev.Handle.Id == currentMaterial->GetId() && prev.Handle.Type == currentMaterial->GetType();

						auto Name = STR_WSTR(prev.Name);
						ImGui::PushID((Name + std::to_string(idx)).c_str());
						if (ImGui::Selectable(Name.c_str(), &selected))
						{
							SetMaterial(prev.Handle);
							changeValue = true;
						}
						ImGui::PopID();

						idx++;
					}

					ImGui::EndPopup();
				}

				ImGui::EndTable();
			}
		}

		return changeValue;

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
		mMeshResource = D_RESOURCE::GetResource<SkeletalMeshResource>(handle, *GetGameObject());

		if (mMeshResource.IsValid())
		{
			mJoints.resize(mMeshResource->GetJointCount());
		}
	}

	void SkeletalMeshRendererComponent::_SetMaterial(ResourceHandle handle)
	{
		mMaterialResource = D_RESOURCE::GetResource<MaterialResource>(handle, *GetGameObject());
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
			skeletonJoint.Xform.Set3x3(Matrix3(skeletonJoint.Rotation) * Matrix3::MakeScale(skeletonJoint.Scale));
		}

		auto xform = skeletonJoint.Xform;
		if (!skeletonJoint.SkeletonRoot)
			xform = parent * xform;

		auto& joint = mJoints[skeletonJoint.MatrixIdx];
		auto withOffset = xform * skeletonJoint.IBM;
		joint.mWorld = withOffset;
		//joint.mWorldIT = InverseTranspose(withOffset.Get3x3());

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
		if (GetGameObject()->GetType() == GameObject::Type::Static)
			return;

		if (!mMeshResource.IsValid())
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin();

		//// Updating joints matrices (indivisual)
		//if (HasAnimation())
		//{
		//	for (int i = 0; i < mSkeleton.size(); i++)
		//	{
		//		auto& joint = mSkeleton[i];

		//		// Update matrix if dirty
		//		if (joint.StaleMatrix)
		//		{
		//			joint.StaleMatrix = false;
		//			joint.Xform.Set3x3(Matrix3(joint.Rotation) * Matrix3::MakeScale(joint.Scale));
		//		}
		//	}
		//}

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		MeshConstants* cb = (MeshConstants*)currentUploadBuff.Map();


		auto world = GetTransform().GetWorld();
		cb->mWorld = Matrix4(world);
		//cb->mWorldIT = InverseTranspose(Matrix3(world));

		// Updating joints matrices on gpu
		auto skeletonRoot = mMeshResource->GetSkeletonRoot();
		if(skeletonRoot)
		{
			//static const size_t kMaxStackDepth = 32;
			//size_t stackIdx = 0;
			//Matrix4 matrixStack[kMaxStackDepth];
			//Matrix4 parentMatrix = Matrix4(world);


			//for (const Mesh::SkeletonJoint* joint = mSkeleton.data(); ; ++joint)
			//{
			//	auto Xform = joint->Xform;
			//	if (!joint->SkeletonRoot)
			//		Xform = parentMatrix * Xform;

			//	// Concatenate the transform with the parent's matrix and update the matrix list
			//	{
			//		// Scoped so that I don't forget that I'm pointing to write-combined memory and
			//		// should not read from it.
			//		auto index = joint->MatrixIdx;
			//		auto& j = mJoints[index];
			//		auto withOffset = Xform * ibms[index];
			//		j.mWorld = withOffset;
			//		//j.mWorldIT = InverseTranspose(withOffset.Get3x3());

			//		Matrix4 tmp = Matrix4(world) * Xform;
			//		Scalar scaleXSqr = LengthSquare((Vector3)tmp.GetX());
			//		Scalar scaleYSqr = LengthSquare((Vector3)tmp.GetY());
			//		Scalar scaleZSqr = LengthSquare((Vector3)tmp.GetZ());
			//		Scalar sphereScale = Sqrt(Max(Max(scaleXSqr, scaleYSqr), scaleZSqr));
			//		D_DEBUG_DRAW::DrawSphere((Vector3)tmp.GetW(), sphereScale * 10.f, {1.f, 0.f, 0.f, 1.f});

			//		/*boundingSphereTransforms[Node->matrixIdx] = ScaleAndTranslation((Vector3)xform.GetW(), sphereScale); */
			//	}

			//	// If the next node will be a descendent, replace the parent matrix with our new matrix
			//	if (joint->hasChildren)
			//	{
			//		// ...but if we have siblings, make sure to backup our current parent matrix on the stack
			//		if (joint->hasSibling)
			//		{
			//			D_ASSERT(stackIdx < kMaxStackDepth, "Overflowed the matrix stack");
			//			matrixStack[stackIdx++] = parentMatrix;
			//		}
			//		parentMatrix = Xform;
			//	}
			//	else if (!joint->hasSibling)
			//	{
			//		// There are no more siblings.  If the stack is empty, we are done.  Otherwise, pop
			//		// a matrix off the stack and continue.
			//		if (stackIdx == 0)
			//			break;

			//		parentMatrix = matrixStack[--stackIdx];
			//	}
			//}

			JointUpdateRecursion(Matrix4(), *skeletonRoot);
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
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Destroy();
		}
		mMeshConstantsGPU.Destroy();
	}

	void SkeletalMeshRendererComponent::CreateGPUBuffers()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));
	}
}
