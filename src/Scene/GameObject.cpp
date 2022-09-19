#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "GameObject.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"
#include "Scene.hpp"
#include "Utils/Serializer.hpp"
#include "EntityComponentSystem/Components/ComponentBase.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"

#include <Core/Uuid.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Renderer/RenderDeviceManager.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_RESOURCE;
using namespace D_ECS_COMP;

namespace Darius::Scene
{
	// Comp name and display name
	D_CONTAINERS::DVector<std::pair<std::string, std::string>> GameObject::RegisteredComponents = D_CONTAINERS::DVector<std::pair<std::string, std::string>>();

	GameObject::GameObject(Uuid uuid, D_ECS::Entity entity) :
		mActive(true),
		mType(Type::Movable),
		mName("GameObject"),
		mUuid(uuid),
		mEntity(entity),
		mStarted(false),
		mDeleted(false),
		mParent(nullptr)
	{
		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

		AddComponent<D_ECS_COMP::TransformComponent>();
	}

	void GameObject::OnDestroy()
	{
		VisitComponents([](auto comp)
			{
				comp->OnDestroy();
			});

		mEntity.each([&](flecs::id compId)
			{
				mEntity.remove(compId);
			});
	}

	void GameObject::Update(D_GRAPHICS::GraphicsContext& context)
	{
		// We won't update constant buffer for static objects
		if (mType == Type::Static)
			return;

		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = mMeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		MeshConstants* cb = (MeshConstants*)currentUploadBuff.Map();

		auto world = GetTransform().GetWorld();
		cb->mWorld = Matrix4(world);
		cb->mWorldIT = InverseTranspose(Matrix3(world));

		currentUploadBuff.Unmap();

		// Uploading
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(mMeshConstantsGPU.GetResource(), 0, currentUploadBuff.GetResource(), 0, currentUploadBuff.GetBufferSize());
		context.TransitionResource(mMeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

	}

#ifdef _D_EDITOR
	bool GameObject::DrawDetails(float params[])
	{
		bool changeValue = false;

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		// Drawing game object header
		char* name = const_cast<char*>(mName.c_str());
		ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight * 0.5f - 10.f);
		ImGui::InputText("##ObjectName", name, 30);
		bool active = mActive;
		ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
		if (ImGui::Checkbox("##Active", &active))
		{
			SetActive(active);
			changeValue = true;
		}
		ImGui::Spacing();

		// Drawing components

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		// Drawing components
		VisitComponents([&](auto comp)
			{
				bool isTransform = dynamic_cast<D_ECS_COMP::TransformComponent*>(comp);

				// Styling component frame
				ImGui::Separator();

				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				bool open = ImGui::TreeNodeEx(comp->GetDisplayName().c_str(), treeNodeFlags);
				ImGui::PopStyleVar();

				if (isTransform)
				{
					std::string a;
					ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				}
				else
				{
					ImGui::SameLine(contentRegionAvailable.x - 2 * lineHeight);

					// Component enabled box
					auto enabled = comp->GetEnabled();
					if (ImGui::Checkbox("##Enabled", &enabled))
					{
						comp->SetEnabled(enabled);
						changeValue = true;
					}
					ImGui::SameLine();
				}

				if (ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL, { lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
				ImGui::PopStyleVar();

				if (ImGui::BeginPopup("ComponentSettings"))
				{

					if (!isTransform && ImGui::MenuItem("Remove component"))
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

		// Component selection popup
		if (ImGui::BeginPopupContextItem("ComponentAdditionPopup", ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			auto& reg = D_WORLD::GetRegistry();

			for (auto const& [compName, compDisplayName] : RegisteredComponents)
			{
				auto compGeneric = reg.component(compName.c_str());
				if (!reg.is_valid(compGeneric))
					continue;

				if (mEntity.has(compGeneric))
					continue;

				if (ImGui::Selectable(compDisplayName.c_str()))
				{
					AddComponent(compName);
				}
			}

			ImGui::EndPopup();
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		float addCompButtonWidth = 100.f;
		ImGui::SameLine(std::max((contentRegionAvailable.x - addCompButtonWidth) / 2, 0.f));

		if (ImGui::Button("Add Component", ImVec2(addCompButtonWidth, 0)))
		{
			ImGui::OpenPopup("ComponentAdditionPopup");
		}

		return changeValue;
	}
#endif // _EDITOR

	void GameObject::SetLocalTransform(Transform const& trans)
	{
		GetComponent<Darius::Scene::ECS::Components::TransformComponent>()->SetLocalTransform(trans);
	}

	Transform const& GameObject::GetLocalTransform() const
	{
		return *mEntity.get<Darius::Scene::ECS::Components::TransformComponent>()->GetDataC();
	}

	void GameObject::VisitComponents(std::function<void(ComponentBase*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException) const
	{
		auto compList = DVector<ComponentBase*>();

		compList.push_back(mEntity.get_mut<TransformComponent>());

		auto& reg = D_WORLD::GetRegistry();

		auto transId = reg.id<TransformComponent>();

		mEntity.each([&](flecs::id compId)
			{
				if (!reg.is_valid(compId))
					return;

				if (transId == compId)
					return;

				bool hh = mEntity.has(compId);
				auto compP = mEntity.get_mut(compId);
				try
				{
					auto comp = reinterpret_cast<ComponentBase*>(compP);
					compList.push_back(comp);

				}
				catch (const D_EXCEPTION::Exception& e)
				{
					if (onException)
						onException(e);
				}
			});

		for (auto comp : compList)
			callback(comp);
	}

	D_ECS_COMP::ComponentBase* GameObject::AddComponent(std::string const& name)
	{
		auto& reg = D_WORLD::GetRegistry();

		auto compT = reg.component(name.c_str());

		auto bb = reg.is_valid(compT);

		mEntity.add(compT);
		auto compP = mEntity.get_mut(compT);

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
		comp->OnDestroy();
		mEntity.remove(compId);
	}

	void GameObject::SetParent(GameObject* newParent)
	{
		auto& reg = D_WORLD::GetRegistry();

		if (!newParent) // Unparent
		{
			if (mParent) // Already has a parent
			{
				mEntity.child_of(D_WORLD::GetRoot());
			}

			mParent = nullptr;
			return;
		}

		// New parent is legit
		mEntity.child_of(newParent->mEntity);
		mParent = newParent;
	}

	void GameObject::RegisterComponent(std::string name, std::string displayName)
	{
		auto& reg = D_WORLD::GetRegistry();
		if (!reg.is_valid(reg.component(name.c_str())))
			throw D_EXCEPTION::Exception("found no component using provided name");

		GameObject::RegisteredComponents.push_back({ name, displayName });
		std::sort(RegisteredComponents.begin(), RegisteredComponents.end(), [](auto first, auto second)
			{
				return first.second < second.second;
			});
	}

	void GameObject::VisitAncestors(std::function<void(GameObject*)> callback) const
	{
		auto current = mParent;

		auto ancestorList = DVector<GameObject*>();

		while (current)
		{
			ancestorList.push_back(current);
			current = current->mParent;
		}

		for (auto anc : ancestorList)
			callback(anc);
	}

	void GameObject::VisitChildren(std::function<void(GameObject*)> callback) const
	{
		auto childrenList = DVector<GameObject*>();

		mEntity.children([&](D_ECS::Entity childEnt)
			{
				childrenList.push_back(D_WORLD::GetGameObject(childEnt));
			});

		for (auto child : childrenList)
			callback(child);
	}

	// TODO: All descendants should be loaded on a list and then iterated, otherwise a descendant may change the hierarchy and invalidates the traversal.
	void GameObject::VisitDescendants(std::function<void(GameObject*)> callback) const
	{
		VisitChildren([&](GameObject* child)
			{
				callback(child);
				child->VisitDescendants(callback);
			});
	}

	UINT GameObject::CountChildren()
	{
		int result = 0;
		VisitChildren([&](GameObject*)
			{
				result++;
			});
		return result;
	}

	void GameObject::SetActive(bool active)
	{
		this->mActive = active;

		if (active)
			VisitComponents([](auto comp)
				{
					comp->OnGameObjectActivate();
				});
		else
			VisitComponents([](auto comp)
				{
					comp->OnGameObjectDeactivate();
				});

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
