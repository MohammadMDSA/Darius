#pragma once

#include "Scene/EntityComponentSystem/CompRef.hpp"
#include "Scene/EntityComponentSystem/ComponentEvent.hpp"
#include "Scene/GameObject.hpp"
#include "Scene/Scene.hpp"

#include <Core/Serialization/Json.hpp>
#include <Core/Signal.hpp>
#include <Core/StringId.hpp>
#include <Core/Uuid.hpp>
#include <Math/VectorMath.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>
#include <Utils/Log.hpp>
#include <Utils/StaticConstructor.hpp>

#include <boost/algorithm/string.hpp>
#include <rttr/registration_friend.h>
#include <rttr/registration.h>
#include <rttr/type.h>

#include <ComponentBase.generated.hpp>

#if _D_EDITOR
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#endif // _D_EDITOR


#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

#if _D_EDITOR
#define _D_H_COMP_DISPLAYNAME_DEC() \
static std::string DisplayName;
#else
#define _D_H_COMP_DISPLAYNAME_DEC()
#endif

#define D_H_COMP_BODY(type, parent, compName, shouldRegister) D_H_COMP_BODY_RAW(type, parent, compName, shouldRegister, false, false)

#define D_H_COMP_BODY_RAW(T, parent, compName, shouldRegister, isBehaviour, receivesUpdates) \
public: \
T(); \
T(D_CORE::Uuid const& uuid); \
using Super = parent; \
using ThisClass = T; \
static constexpr INLINE std::string ClassName() { return D_NAMEOF(T); } \
static INLINE D_ECS::ComponentEntry GetComponentEntryStatic() { return D_WORLD::GetComponentEntity(T::GetComponentNameStatic()); } \
virtual INLINE std::string GetDisplayName() const override { return T::DisplayName; } \
virtual INLINE D_CORE::StringId GetComponentName() const override { return T::GetComponentNameStatic(); } \
static INLINE D_CORE::StringId GetComponentNameStatic() { return CompName; } \
virtual INLINE rttr::type GetComponentType() const override { return rttr::type::get<T>(); }; \
INLINE operator D_ECS::CompRef<T>() { return D_ECS::CompRef<T>(GetGameObject()); } \
static void StaticConstructor() \
{ \
    /* Registering component*/ \
    if(sInit) \
        return; \
    D_LOG_INFO("Registering " D_NAMEOF(T) " child of " D_NAMEOF(parent)); \
    parent::StaticConstructor(); \
	CompName = D_CORE::StringId(D_NAMEOF(T), D_ECS_COMP::NameDatabase); \
    auto comp = D_WORLD::RegisterComponent<T, parent>(); \
    D_CONTAINERS::DVector<std::string> splitted; \
    boost::split(splitted, compName, boost::is_any_of("/")); \
    T::DisplayName = splitted[splitted.size() - 1]; \
    if(shouldRegister) \
        D_SCENE::GameObject::RegisterComponent(CompName, splitted); \
    if(isBehaviour) \
    { \
        D_SCENE::GameObject::RegisterBehaviourComponent(comp); \
        if(receivesUpdates) \
        { \
            D_WORLD::RegisterComponentUpdater(&ComponentUpdater); \
            D_WORLD::RegisterComponentLateUpdater(&ComponentLateUpdater); \
        } \
    } \
    sInit = true; \
} \
\
static void StaticDistructor() \
{} \
private: \
static D_CORE::StringId CompName; \
static bool sInit; \
_D_H_COMP_DISPLAYNAME_DEC()

#if _D_EDITOR

#define _D_H_COMP_DISPLAYNAME_DEF(type) \
std::string type::DisplayName = "";

#else

#define _D_H_COMP_DISPLAYNAME_DEF(type)

#endif // _D_EDITOR


#define D_H_COMP_DEF(type) \
D_CORE::StringId type::CompName; \
bool type::sInit = false; \
_D_H_COMP_DISPLAYNAME_DEF(type)

#define D_H_COMP_DEFAULT_CONSTRUCTOR_DEF_PAR(type, parent) \
type::type() : \
    parent() {} \
type::type(D_CORE::Uuid const& uuid) : \
    parent(uuid) {}

#define D_H_COMP_DEFAULT_CONSTRUCTOR_DEF(type) D_H_COMP_DEFAULT_CONSTRUCTOR_DEF_PAR(type, ComponentBase)

#if _D_EDITOR

#define D_H_COMPONENT_DRAG_DROP_DESTINATION(CompType, setter) \
if (ImGui::BeginDragDropTarget()) \
{ \
	ImGuiPayload const* imPayload = ImGui::GetDragDropPayload(); \
	auto payload = reinterpret_cast<Darius::Utils::BaseDragDropPayloadContent const*>(imPayload->Data); \
	if (payload && payload->PayloadType != D_UTILS::BaseDragDropPayloadContent::Type::Invalid && payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::Component, CompType::GetComponentNameStatic())) \
	{ \
		if (ImGuiPayload const* acceptedPayload = ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_GAMEOBJECT)) \
		{ \
			auto gameObject = reinterpret_cast<D_SCENE::GameObjectDragDropPayloadContent const*>(acceptedPayload->Data)->GameObjectRef; \
			setter(gameObject->GetComponent<CompType>()); \
		} \
	} \
	ImGui::EndDragDropTarget(); \
}

#define D_H_GAMEOBJECT_DRAG_DROP_DESTINATION(setter) \
{ \
	if (ImGui::BeginDragDropTarget()) \
	{ \
		ImGuiPayload const* imPayload = ImGui::GetDragDropPayload(); \
		auto payload = reinterpret_cast<Darius::Utils::BaseDragDropPayloadContent const*>(imPayload->Data); \
		if (payload && payload->PayloadType != D_UTILS::BaseDragDropPayloadContent::Type::Invalid && payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::GameObject, D_CORE::StringId(D_PAYLOAD_TYPE_GAMEOBJECT, D_SCENE::GameObject::NameDatabase))) \
		{ \
			if (ImGuiPayload const* acceptedPayload = ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_GAMEOBJECT)) \
			{ \
				auto gameObject = reinterpret_cast<D_SCENE::GameObjectDragDropPayloadContent const*>(acceptedPayload->Data)->GameObjectRef; \
				setter(gameObject); \
			} \
		} \
		ImGui::EndDragDropTarget(); \
	} \
}

#define D_H_GAMEOBJECT_SELECTION_DRAW(prop, setter) \
{ \
	bool isMissing; \
	std::string displayText; \
	if (prop.IsValid(isMissing)) \
		displayText = prop->GetName(); \
	else if (isMissing) \
		displayText = "Missing (Game Object)"; \
	else \
		displayText = "<None>"; \
	\
	auto availableSpace = ImGui::GetContentRegionAvail(); \
	auto selectionWidth = 20.f; \
	\
	ImGui::Button((displayText + std::string("##"#prop)).c_str(), ImVec2(availableSpace.x - 2 * selectionWidth - 10.f, 0)); \
	if(ImGui::IsItemClicked(ImGuiMouseButton_Right)) \
		setter(nullptr); \
	\
	D_H_GAMEOBJECT_DRAG_DROP_DESTINATION(setter); \
}

#define D_H_GAMEOBJECT_SELECTION_DRAW_SIMPLE(prop) \
D_H_GAMEOBJECT_SELECTION_DRAW(prop, [&](D_SCENE::GameObject* go) { prop = go; })

#define D_H_COMPONENT_SELECTION_DRAW(compType, prop, setter) \
{ \
	ImGui::BeginGroup(); \
	bool isMissing = prop.IsMissing(); \
	std::string displayText; \
	if (prop.IsValid()) \
		displayText = prop.GetGameObject()->GetName(); \
	else if (isMissing) \
		displayText = "Missing"; \
	else \
		displayText = "<None>"; \
	auto availableSpace = ImGui::GetContentRegionAvail(); \
	auto selectionWidth = 20.f; \
\
	ImGui::Button(displayText.c_str(), ImVec2(availableSpace.x - 2 * selectionWidth - 10.f, 0)); \
	if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) \
		setter(nullptr); \
\
	D_H_COMPONENT_DRAG_DROP_DESTINATION(compType, setter); \
	ImGui::EndGroup(); \
}

#endif // _D_EDITOR


namespace Darius::Scene
{
	class SceneManager;
}

namespace Darius::ResourceManager
{
	struct ResourceHandle;
}

namespace Darius::Scene::ECS::Components
{
	class ComponentBase;
}

namespace Darius::Scene::ECS
{
	template<typename ...T>
	class GenericComponentSignal
	{
	public:
		typedef GenericComponentSignalSlot<void, T...> Slot;

		GenericComponentSignal()
		{
			mSignal = std::make_shared<D_CORE::Signal<void(T...)>>();
		}

	public:
		template<class COMP, typename FUNCTION>
		INLINE D_CORE::SignalConnection ConnectComponent(COMP* comp, FUNCTION func)
		{
			// Checking if FUNCTION is a member function
			D_STATIC_ASSERT(std::is_member_function_pointer<FUNCTION>::value);
			// Checking if COMP is a component type
			using conv = std::is_convertible<COMP*, D_ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);
			// Checking if FUNCTION signature is the same with the signal signature
			D_STATIC_ASSERT(std::is_same_v<FUNCTION, void(COMP::*)(T...)>);
			D_ASSERT(comp);

			Slot slot;
			slot.Func = [inner = func](void* obj, T... args)
				{
					return (reinterpret_cast<COMP*>(obj)->*inner)(args...);
				};
			slot.Comp = UntypedCompRef(comp->GetGameObject()->GetEntity(), comp->GetComponentEntry());
			return mSignal->connect(slot);
		}

		INLINE void operator() (T... params) const
		{
			(*mSignal)(params...);
		}

	private:
		std::shared_ptr<D_CORE::Signal<void(T...)>> mSignal;
	};

#define D_H_SIGNAL_COMP(ClassName) \
	class ClassName : public D_ECS::GenericComponentSignal<> { }

#define D_H_SIGNAL_COMP_ONE_PARAM(ClassName, Param1Type, Param1Name) \
	class ClassName : public D_ECS::GenericComponentSignal<Param1Type> { }

#define D_H_SIGNAL_COMP_TWO_PARAM(ClassName, Param1Type, Param1Name, Param2Type, Param2Name) \
	class ClassName : public D_ECS::GenericComponentSignal<Param1Type, Param2Type> { }

#define D_H_SIGNAL_COMP_THREE_PARAM(ClassName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name) \
	class ClassName : public D_ECS::GenericComponentSignal<Param1Type, Param2Type, Param3Type> { }

#define D_H_SIGNAL_COMP_FOUR_PARAM(ClassName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, Param4Type, Param4Name) \
	class ClassName : public D_ECS::GenericComponentSignal<Param1Type, Param2Type, Param3Type, Param4Type> { }

#define D_H_SIGNAL_COMP_FIVE_PARAM(ClassName, Param1Type, Param1Name, Param2Type, Param2Name, Param3Type, Param3Name, Param4Type, Param4Name, Param5Type, Param5Name) \
	class ClassName : public D_ECS::GenericComponentSignal<Param1Type, Param2Type, Param3Type, Param4Type, Param5Type> { }

}


namespace Darius::Scene::ECS::Components
{
	extern D_CORE::StringIdDatabase NameDatabase;

	D_H_SIGNAL_COMP_ONE_PARAM(ComponentChangeSignalType, ComponentBase*, thisComponent);

	class DClass(Serialize) ComponentBase
	{
		GENERATED_BODY();

	public:

		ComponentBase();
		ComponentBase(D_CORE::Uuid const& uuid);

#ifdef _D_EDITOR
		virtual INLINE bool         DrawDetails(float[]) { return false; }
		virtual INLINE void         OnGizmo() const { }
#endif
		virtual INLINE D_CORE::StringId GetComponentName() const { D_ASSERT_NOENTRY(); return D_CORE::StringId("", NameDatabase); }
		virtual INLINE std::string  GetDisplayName() const { return ""; }
		virtual INLINE rttr::type   GetComponentType() const { return rttr::type::get<ComponentBase>(); };
		INLINE ComponentEntry		GetComponentEntry() const { return D_WORLD::GetComponentEntity(GetComponentName()); }


		virtual INLINE void         Start() { }
		virtual INLINE void         Awake() { }
		virtual INLINE void         OnDestroy() { }
		virtual INLINE void         OnPreDestroy() { }
		virtual INLINE void         OnSerialized() const { }
		virtual INLINE void         OnDeserialized() { }

#if _D_EDITOR
		virtual void                OnPostComponentAddInEditor() { }
		virtual void                OnPreComponentRemovInEditor() { }
#endif // _D_EDITOR


		// GameObject Events
		virtual INLINE void         OnActivate() {}
		virtual INLINE void         OnDeactivate() {}

		virtual INLINE void         Update(float) { };
		virtual INLINE void         LateUpdate(float) { };


		void						SetDirty();
		virtual INLINE bool			IsDirty() const { return mDirty; }
		bool						CanChange() const;

		bool                        IsActive() const;
		INLINE bool                 IsEnabled() const { return mEnabled; }
		INLINE bool                 IsDestroyed() const { return mDestroyed; }
		INLINE bool                 IsStarted() const { return mStarted; }
		INLINE GameObject*			GetGameObject() const { return mGameObject; }
		INLINE D_CORE::Uuid const&	GetUuid() const { return mUuid; }

		virtual INLINE bool         IsDisableable() const { return true; }

		virtual void                SetEnable(bool value);

		D_MATH::TransformComponent* GetTransform() const;

		static INLINE std::string   ClassName() { return "ComponentBase"; }

		static void                 StaticConstructor()
		{
			if (sInit)
				return;

			D_WORLD::RegisterComponent<ComponentBase>();
			sInit = true;
		}

		static void                 ComponentUpdater(float, D_ECS::ECSRegistry&) {  }
		static void                 ComponentLateUpdater(float, D_ECS::ECSRegistry&) {  }

		static void                 StaticDestructor()
		{
		}

	protected:

		void                        SetClean() { mDirty = false; }


	public:

		ComponentChangeSignalType  mChangeSignal;

#if _D_EDITOR
		static D_CORE::Signal<void(D_FILE::Path const&, Darius::ResourceManager::ResourceHandle const&, bool selected)> RequestPathChange;
#endif // _D_EDITOR

	private:
		friend class Darius::Scene::GameObject;
		friend class Darius::Scene::SceneManager;


		DField(Serialize, NotAnimate)
		D_CORE::Uuid                mUuid;

		DField()
		Darius::Scene::GameObject* mGameObject;

		DField()
		bool                        mStarted;

		DField(Serialize)
		bool                        mEnabled;

		DField()
		bool                        mDestroyed;

		bool						mDirty;

		static bool                 sInit;
		static D_CORE::StringId		CompName;
#if _D_EDITOR
		static std::string          DisplayName;
#endif
	};


	INLINE void ComponentBase::SetDirty()
	{
		if (CanChange())
			mDirty = true;
	}

	INLINE bool ComponentBase::CanChange() const
	{
#ifdef _D_EDITOR
		if (!D_WORLD::IsRunning() || (GetGameObject() && GetGameObject()->GetType() != D_SCENE::GameObject::Type::Static))
#else
		if (GetGameObject() && GetGameObject()->GetType() != D_SCENE::GameObject::Type::Static)
#endif
			return true;
		return false;
	}

}

INLINE D_CORE::StringId operator ""_Comp(char const* str, std::size_t)
{
	return D_CORE::StringId(str, D_ECS_COMP::NameDatabase);
}

namespace rttr
{
	template<typename T>
	struct wrapper_mapper<D_ECS::CompRef<T>>
	{
		using wrapped_type = D_SERIALIZATION::UuidWrapper;
		using type = D_ECS::CompRef<T>;

		INLINE static wrapped_type get(type const& obj)
		{
			if (!obj.IsValid())
				return { D_CORE::Uuid(), D_SERIALIZATION_UUID_PARAM_GAMEOBJECT };

			return { obj.GetGameObject()->GetUuid(), D_SERIALIZATION_UUID_PARAM_GAMEOBJECT};
		}

		INLINE static type create(wrapped_type const& value)
		{
			auto go = D_WORLD::GetGameObject(value.Uuid);
			return D_ECS::CompRef<T>(go);
		}

		template<typename U>
		INLINE static D_ECS::CompRef<U> convert(type const& source, bool& ok)
		{
			if (!source.IsValid())
			{
				ok = false;
				return D_ECS::CompRef<U>();
			}

			type& src = const_cast<type&>(source);
			if (auto conv = dynamic_cast<U*>(src.Get()))
			{
				auto ent = source.GetEntity();
				if (ent.has<U>())
				{
					ok = true;
					return D_ECS::CompRef<U>(ent);
				}
			}

			ok = false;
			return D_ECS::CompRef<U>();
		}
	};
}

File_ComponentBase_GENERATED