#include "pch.hpp"

#include "GameObject.hpp"
#include "GameObjectRef.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"
#include "Scene.hpp"
#include "EntityComponentSystem/Components/ComponentBase.hpp"
#include "EntityComponentSystem/Components/BehaviourComponent.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"

#include <Core/Serialization/Copyable.hpp>
#include <ResourceManager/ResourceManager.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#include <imgui_internal.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#endif

#include <GameObject.sgenerated.hpp>

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_ECS_COMP;
using namespace D_MATH;
using namespace D_RESOURCE;

namespace Darius::Scene
{
	// Comp name and display name
	D_CONTAINERS::DMap<std::string, GameObject::ComponentAddressNode> GameObject::RegisteredComponents = D_CONTAINERS::DMap<std::string, GameObject::ComponentAddressNode>();
	D_CONTAINERS::DSet<D_ECS::EntityId> GameObject::RegisteredBehaviours = D_CONTAINERS::DSet<D_ECS::EntityId>();
	D_CONTAINERS::DSet<D_CORE::StringId> GameObject::RegisteredComponentNames = D_CONTAINERS::DSet<D_CORE::StringId>();

	GameObject::GameObject(D_CORE::Uuid const& uuid, D_ECS::Entity entity, bool inScene) :
		mActive(true),
		mType(Type::Movable),
		mName("GameObject"),
		mUuid(uuid),
		mEntity(entity),
		mStarted(false),
		mDeleted(false),
		mParent(nullptr),
		mAwake(false),
		mPrefab(Uuid()),
		mInScene(inScene)
	{
		AddComponent<D_MATH::TransformComponent>();
	}

	void GameObject::OnDestroy()
	{
		if(!IsInScene())
			return;

		VisitComponents([&](auto comp)
			{
				RemoveComponentRoutine(comp);
			});
	}

	void GameObject::OnPreDestroy()
	{
		if(!IsInScene())
			return;

		VisitComponents([](auto comp)
			{
				comp->OnPreDestroy();
			});
	}

#ifdef _D_EDITOR
	bool GameObject::DrawDetails(float params[])
	{
		bool changeValue = false;

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		// Drawing game object header
		ImGui::SetNextItemWidth(contentRegionAvailable.x - lineHeight * 0.5f - 10.f);

		// Setting GameObject name
		{
			char name[1000];
			memset(name, 0, 1000 * sizeof(char));
			std::strcpy(name, mName.string());
			if(ImGui::InputText("##ObjectName", name, 30))
				mName = StringId(name);
		}

		bool active = mActive;
		ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
		if(ImGui::Checkbox("##Active", &active))
		{
			SetActive(active);
			changeValue = true;
		}

		bool isStatic = GetType() == Type::Static;
		if(ImGui::Checkbox("Static", &isStatic))
		{
			SetType(isStatic ? Type::Static : Type::Movable);
			changeValue = true;
		}

		ImGui::Spacing();

		// Show parent prefab
		if(!mPrefab.is_nil() && mPrefab != mUuid)
		{

			ImGui::Text("Prefab:");
			ImGui::SameLine(0, 50.f);
			auto prefabGo = D_WORLD::GetGameObject(mPrefab);
			ImGui::Button(prefabGo->GetNameId().string());

			ImGui::Spacing();
		}

		// Drawing components
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		// Drawing components
		VisitComponents([&](auto comp)
			{
				if(!comp || !comp->GetGameObject())
					return;

				auto label = comp->GetDisplayName();

				if(label == "")
					return;

				bool isTransform = dynamic_cast<D_MATH::TransformComponent*>(comp);

				// Styling component frame
				ImGui::Separator();

				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 {4, 4});

				bool open = ImGui::TreeNodeEx(label.c_str(), treeNodeFlags);
				ImGui::PopStyleVar();


				// Component enabled box
				{
					auto canDisable = comp->IsDisableable();

					if(canDisable)
					{
						ImGui::SameLine(contentRegionAvailable.x - 2 * lineHeight);
						auto enabled = comp->IsEnabled();
						if(ImGui::Checkbox("##Enabled", &enabled))
						{
							comp->SetEnable(enabled);
							changeValue = true;
						}
						ImGui::SameLine();
					}
					else
					{
						ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

					}

				}

				if(ImGui::Button(ICON_FA_ELLIPSIS_VERTICAL, {lineHeight, lineHeight}))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
				ImGui::PopStyleVar();

				bool deleted = false;
				if(ImGui::BeginPopup("ComponentSettings"))
				{

					if(!isTransform && ImGui::MenuItem("Remove component"))
					{
						comp->OnPreComponentRemovInEditor();
						RemoveComponent(comp);
						deleted = true;
					}

					ImGui::Separator();

					if(ImGui::MenuItem("Copy Component"))
					{
						//ComponentBase::GlobalComponentClipboard.SetData()
					}

					ImGui::EndPopup();
				}

				if(open)
				{
					if(!deleted)
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
		if(ImGui::BeginPopup("ComponentAdditionPopup", ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			DrawComponentNameContext(RegisteredComponents);
			ImGui::EndPopup();
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		float addCompButtonWidth = 100.f;
		ImGui::SameLine(std::max((contentRegionAvailable.x - addCompButtonWidth) / 2, 0.f));

		if(ImGui::Button("Add Component", ImVec2(addCompButtonWidth, 0)))
		{
			ImGui::OpenPopup("ComponentAdditionPopup");
		}

		return changeValue;
	}

	void GameObject::DrawComponentNameContext(D_CONTAINERS::DMap<std::string, ComponentAddressNode> const& componentNameTree)
	{

		for(auto const& [compDisplay, compGroup] : componentNameTree)
		{

			if(compGroup.IsBranch)
			{
				if(compGroup.ChildrenNameMap.size() && ImGui::BeginMenu(compDisplay.c_str()))
				{
					DrawComponentNameContext(compGroup.ChildrenNameMap);
					ImGui::EndMenu();
				}

				continue;
			}

			auto compGeneric = D_WORLD::GetComponentEntity(compGroup.ComponentName);
			if(!D_WORLD::IsIdValid(compGeneric))
				continue;

			if(mEntity.has(compGeneric))
				continue;

			if(ImGui::MenuItem(compDisplay.c_str()))
			{
				auto addedComp = AddComponent(compGroup.ComponentName);
				addedComp->OnPostComponentAddInEditor();
			}
		}
	}

	void GameObject::OnGizmo() const
	{
		VisitComponents([](auto comp)
			{
				comp->OnGizmo();
			});
	}

#endif // _EDITOR

	void GameObject::VisitBehaviourComponents(std::function<void(Darius::Scene::ECS::Components::BehaviourComponent*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException) const
	{
		auto compList = DVector<BehaviourComponent*>();
		mEntity.each([&](flecs::id compId)
			{
				if(!D_WORLD::IsIdValid(compId))
					return;

				if(!RegisteredBehaviours.contains(compId))
					return;

				auto compP = const_cast<void*>(mEntity.get(compId));
				try
				{
					auto comp = reinterpret_cast<BehaviourComponent*>(compP);
					compList.push_back(comp);

				}
				catch(const D_EXCEPTION::Exception& e)
				{
					if(onException)
						onException(e);
				}
			});

		for(auto comp : compList)
			callback(comp);
	}

	void GameObject::VisitComponents(std::function<void(ComponentBase*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException) const
	{
		auto compList = DVector<ComponentBase*>();

		compList.push_back(const_cast<D_MATH::TransformComponent*>(mEntity.get<D_MATH::TransformComponent>()));

		static auto transId = D_WORLD::GetTypeId<D_MATH::TransformComponent>();

		mEntity.each([&](flecs::id compId)
			{
				if(!D_WORLD::IsIdValid(compId))
					return;

				if(transId == compId || !mEntity.has(compId))
					return;

				auto compP = const_cast<void*>(mEntity.get(compId));
				try
				{
					auto comp = reinterpret_cast<ComponentBase*>(compP);
					compList.push_back(comp);

				}
				catch(const D_EXCEPTION::Exception& e)
				{
					if(onException)
						onException(e);
				}
			});

		for(auto comp : compList)
		{
			D_VERIFY(comp);
			callback(comp);
		}
	}

	D_ECS_COMP::ComponentBase* GameObject::AddComponent(StringId const& name)
	{
		if(!RegisteredComponentNames.contains(name))
			return nullptr;

		auto compT = D_WORLD::GetComponentEntity(name);

		PreEntityEdit();
		mEntity.add(compT);
		PostEntityEdit();
		auto compP = const_cast<void*>(mEntity.get(compT));

		auto ref = reinterpret_cast<ComponentBase*>(compP);
		AddComponentRoutine(ref);

		return ref;
	}

	void GameObject::AddComponentRoutine(Darius::Scene::ECS::Components::ComponentBase* comp)
	{
		if(!D_VERIFY(comp))
			return;

		comp->mGameObject = this;

		if(mAwake)
		{
			comp->Awake();
			if(comp->IsActive())
				comp->OnActivate();
		}

		if(mStarted && comp->IsActive())
		{
			comp->mStarted = true;
			comp->Start();
		}

		OnComponentAdd(this, comp);
		OnComponentSetChange(this);
	}

	void GameObject::RemoveComponentRoutine(Darius::Scene::ECS::Components::ComponentBase* comp)
	{
		comp->OnDestroy();
		comp->mDestroyed = true;

		OnComponentRemove(this, comp);
		OnComponentSetChange(this);
	}

	void GameObject::Start()
	{
		if(!IsInScene() || mStarted)
			return;

		VisitComponents([](ComponentBase* comp)
			{
				if(!comp->IsActive())
					return;
				comp->mStarted = true;
				comp->Start();
			});

		mStarted = true;
	}

	void GameObject::Awake()
	{
		if(!IsInScene() || mAwake)
			return;

		mAwake = true;

		VisitComponents([](ComponentBase* comp)
			{
				comp->Awake();
				if(comp->IsActive())
					comp->OnActivate();
			});
	}

	bool GameObject::IsActive() const
	{
		if(!IsSelfActive())
			return false;
		auto parent = GetParent();
		if(parent)
			return parent->IsActive();
		return true;
	}

	void GameObject::RemoveComponent(D_ECS_COMP::ComponentBase* comp)
	{
		auto compId = D_WORLD::GetComponentEntity(comp->GetComponentName());

		// Abort if transform
		if(D_WORLD::GetTypeId<D_MATH::TransformComponent>() == compId)
		{
			D_LOG_WARN("Removal of Transform Component from a GameObject is not possible.");
			return;
		}

		RemoveComponentRoutine(comp);
		mEntity.remove(compId);
	}

	bool GameObject::HasComponent(StringId const& compName) const
	{
		if(!RegisteredComponentNames.contains(compName))
			return false;

		auto compT = D_WORLD::GetComponentEntity(compName);

		return mEntity.has(compT);
	}

	D_ECS_COMP::ComponentBase* GameObject::GetComponent(StringId const& compName) const
	{
		if(!RegisteredComponentNames.contains(compName))
			return nullptr;

		auto compT = D_WORLD::GetComponentEntity(compName);


		return reinterpret_cast<D_ECS_COMP::ComponentBase*>(const_cast<void*>(mEntity.get(compT)));
	}

	bool GameObject::CanAttachTo(GameObject const* parent) const
	{
		if(!parent)
			return true;

		if(parent == this)
			return false;

		bool result = true;
		parent->VisitAncestors([&](GameObject* parent)
			{
				if(parent == this)
					result = false;
			});

		return result;
	}

	void GameObject::SetParent(GameObject* newParent, AttachmentType attachmentType)
	{
		if(!D_VERIFY(CanAttachTo(newParent)))
			D_LOG_WARN("Circular pattern in hierarchy");

		if(!newParent || !newParent->IsValid()) // Unparent
		{
			if(mParent) // Already has a parent
			{
				mEntity.child_of(D_WORLD::GetRoot());
			}

			if(attachmentType == AttachmentType::KeepWorld)
			{
				auto trans = GetTransform();
				auto world = trans->GetWorld();
				mParent = nullptr;
				trans->SetWorld(world);
			}
			else
				mParent = nullptr;

			return;
		}

		// New parent is legit
		mEntity.child_of(newParent->mEntity);

		if(attachmentType == AttachmentType::KeepWorld)
		{
			auto trans = GetTransform();
			auto world = trans->GetWorld();
			mParent = newParent;
			trans->SetWorld(world);
		}
		else
			mParent = newParent;

	}

	void GameObject::VisitAncestors(std::function<void(GameObject*)> callback) const
	{
		auto current = mParent;

		auto ancestorList = DVector<GameObject*>();

		while(current)
		{
			ancestorList.push_back(current);
			current = current->mParent;
		}

		for(auto anc : ancestorList)
			callback(anc);
	}

	void GameObject::VisitChildren(std::function<void(GameObject*)> callback) const
	{
		auto childrenList = DVector<GameObject*>();

		mEntity.children([&](D_ECS::Entity childEnt)
			{
				childrenList.push_back(D_WORLD::GetGameObject(childEnt));
			});

		for(auto child : childrenList)
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

		if(active)
			VisitComponents([&](auto comp)
				{
					comp->OnActivate();

					if(comp->mStarted || !mStarted)
						return;

					comp->mStarted = true;
					comp->Start();

				});
		else
			VisitComponents([](auto comp)
				{
					comp->OnDeactivate();
				});

	}

	void GameObject::RegisterComponent(StringId const& name, D_CONTAINERS::DVector<std::string>& displayName)
	{
		if(!D_WORLD::IsIdValid(D_WORLD::GetComponentEntity(name)))
			throw D_EXCEPTION::Exception("Found no component using provided name");

		RegisteredComponentNames.insert(name);

		// Add component name to names list respecting categories
		auto currentLevel = &RegisteredComponents;
		auto& splitted = displayName;
		for(int i = 0; i < splitted.size(); i++)
		{
			// This is the last part of name (what we want to show)
			if(i == splitted.size() - 1)
			{
				ComponentAddressNode node;
				node.ComponentName = name;
				node.IsBranch = false;

				currentLevel->insert({splitted[i], node});
				break;
			}

			// Add to a child cat
			if(!currentLevel->contains(splitted[i]))
			{
				ComponentAddressNode node;
				node.IsBranch = true;
				currentLevel->insert({splitted[i], node});
			}
			currentLevel = &currentLevel->at(splitted[i]).ChildrenNameMap;
		}

	}

	void GameObject::PreEntityEdit()
	{
		//D_WORLD::SetDeferEnable(false);
	}

	void GameObject::PostEntityEdit()
	{
		//D_WORLD::SetDeferEnable(true);
	}

	void GameObject::RegisterBehaviourComponent(D_ECS::EntityId compId)
	{
		RegisteredBehaviours.insert(compId);
	}

	bool GameObject::Release()
	{
		if(IsInScene())
			D_WORLD::DeleteGameObject(this);
		return true;
	}

	DVector<ComponentBase*> GameObject::GetComponents(bool sorted) const
	{
		DVector<ComponentBase*> result;

		VisitComponents([&result](ComponentBase* comp)
			{
				result.push_back(comp);
			});

		// Sort components based on their display name
		if(sorted)
		{
			std::sort(result.begin(), result.end(), [](ComponentBase const* a, ComponentBase const* b)
				{
					return a->GetDisplayName() < b->GetDisplayName();
				});
		}

		return result;
	}

#if _D_EDITOR
	void GameObject::Copy(bool maintainContext, D_SERIALIZATION::Json& serialized) const
	{
		D_SERIALIZATION::Json goData;
		D_WORLD::DumpGameObject(this, goData, maintainContext);
		serialized = D_SERIALIZATION::Json();
		serialized["Type"] = "GameObject";
		serialized["Data"] = goData;
	}
#endif
}
