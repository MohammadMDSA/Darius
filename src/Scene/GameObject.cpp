#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "GameObject.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"
#include "Scene.hpp"
#include "Serialization/Serializer.hpp"
#include "EntityComponentSystem/Components/ComponentBase.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"

#include <Core/Uuid.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Renderer/RenderDeviceManager.hpp>

#include <imgui.h>

using namespace D_RESOURCE;
using namespace D_ECS_COMP;

namespace Darius::Scene
{

	GameObject::GameObject(Uuid uuid, D_ECS::Entity entity) :
		mActive(true),
		mType(Type::Movable),
		mName("GameObject"),
		mUuid(uuid),
		mEntity(entity),
		mStarted(false)
	{
		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

		AddComponent<D_ECS_COMP::TransformComponent>();
	}

	GameObject::~GameObject()
	{
		mEntity.each([&](flecs::id compId)
			{
				mEntity.remove(compId);
			});
	}

	void GameObject::Update(D_GRAPHICS::GraphicsContext& context, float deltaTime)
	{
		// We won't update constant buffer for static objects
		if (mType == Type::Static)
			return;

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		MeshConstants* cb = (MeshConstants*)currentUploadBuff.Map();

		auto world = GetTransform()->GetWorld();
		cb->mWorld = Matrix4(world);
		cb->mWorldIT = InverseTranspose(Matrix3(world));

		currentUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMeshConstantsGPU.GetResource(), 0, mMeshConstantsCPU->GetResource(), 0, mMeshConstantsCPU->GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

#ifdef _D_EDITOR
	bool GameObject::DrawDetails(float params[])
	{
		bool changeValue = false;
		auto& reg = D_WORLD::GetRegistry();

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		// Drawing components
		VisitComponents([&](auto comp)
			{
				// Styling component frame

				ImGui::Separator();
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				bool open = ImGui::TreeNodeEx(comp->GetComponentName().c_str(), treeNodeFlags);
				ImGui::PopStyleVar();
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
				ImGui::PopStyleVar();

				if (ImGui::BeginPopup("ComponentSettings"))
				{
					auto transComp = dynamic_cast<D_ECS_COMP::TransformComponent*>(comp);
					if (!transComp && ImGui::MenuItem("Remove component"))
						RemoveComponent(comp);

					ImGui::EndPopup();
				}

				if (open)
				{
					changeValue |= D_SCENE_DET_DRAW::DrawDetails(*comp, nullptr);
					ImGui::TreePop();
				}
				ImGui::Spacing();
				ImGui::Spacing();

			},
			[](auto const& ex)
			{
				D_LOG_ERROR("Error drawing component with error: " << ex.what());
			});



		return changeValue;
	}
#endif // _EDITOR

	void GameObject::SetTransform(Transform const& trans)
	{
		GetComponent<Darius::Scene::ECS::Components::TransformComponent>()->SetTransform(trans);
	}

	Transform const* GameObject::GetTransform() const
	{
		auto ff = mEntity.get<Darius::Scene::ECS::Components::TransformComponent>()->GetData();
		return ff;
	}

	void GameObject::VisitComponents(std::function<void(ComponentBase*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException) const
	{

		callback(mEntity.get_mut<TransformComponent>());

		auto& reg = D_WORLD::GetRegistry();

		auto transId = reg.id<TransformComponent>();

		mEntity.each([&](flecs::id compId)
			{
				if (!reg.is_valid(compId))
					return;

				if (transId == compId)
					return;

				auto compP = mEntity.get_mut(compId);
				try
				{
					auto comp = reinterpret_cast<ComponentBase*>(compP);
					callback(comp);

				}
				catch (const D_EXCEPTION::Exception& e)
				{
					if (onException)
						onException(e);
				}
			});
	}

	D_ECS_COMP::ComponentBase* GameObject::AddComponent(std::string const& name)
	{
		auto& reg = D_WORLD::GetRegistry();

		auto compT = reg.component(name.c_str());

		auto compEnt = mEntity.add(compT);
		auto compP = mEntity.get_mut(compEnt);

		auto ref = reinterpret_cast<ComponentBase*>(compP);
		AddComponentRoutine(ref);
		return ref;
	}

	void GameObject::AddComponentRoutine(Darius::Scene::ECS::Components::ComponentBase* comp)
	{
		comp->mGameObject = this;
		if (mStarted)
			comp->Start();
	}

	void GameObject::Start()
	{
		if (mStarted)
			return;

		VisitComponents([](auto comp)
			{
				comp->Start();
			});

		mStarted = true;
	}

	void GameObject::RemoveComponent(D_ECS_COMP::ComponentBase* comp)
	{
		auto& reg = D_WORLD::GetRegistry();
		auto compId = reg.component(comp->GetComponentName().c_str());
		
		// Abort if transform
		if (reg.id<D_ECS_COMP::TransformComponent>() == compId)
			return;

		mEntity.remove(compId);
	}

	void to_json(D_SERIALIZATION::Json& j, const GameObject& value) {
		D_H_SERIALIZE(Active);
		D_H_SERIALIZE(Name);
		D_H_SERIALIZE(Type);
		D_CORE::to_json(j["Uuid"], value.mUuid);
	}

	void from_json(const D_SERIALIZATION::Json& j, GameObject& value) {
		D_H_DESERIALIZE(Active);
		D_H_DESERIALIZE(Name);
		D_H_DESERIALIZE(Type);
	}

}
