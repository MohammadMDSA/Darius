#pragma once

#include "EntityComponentSystem/Entity.hpp"

#include <Core/StringId.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/RefCounting/Ref.hpp>
#include <Core/Uuid.hpp>
#include <Core/Serialization/Json.hpp>
#include <Core/Serialization/Copyable.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Core/Containers/Set.hpp>
#include <Core/Signal.hpp>
#include <Math/VectorMath.hpp>
#include <Utils/Detailed.hpp>

#include <rttr/registration.h>
#include <rttr/registration_friend.h>

#include <map>
#include <functional>
#include <type_traits>

#include <GameObject.generated.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene::ECS::Components
{
	class ComponentBase;
	class BehaviourComponent;
}

namespace Darius::Math
{
	class TransformComponent;
}

namespace Darius::Core::Memory
{
	template<class T, bool ThreadSafe>
	class PagedAllocator;
}

namespace Darius::Scene
{
	class SceneManager;

	class DClass(Serialize[Name]) GameObject sealed : public Detailed, public D_CORE::Counted
#if _D_EDITOR
		, public D_SERIALIZATION::ICopyable
#endif
	{

	public:

		enum class DEnum(Serialize) Type
		{
			Static,
			Movable
		};

		enum class DEnum() AttachmentType
		{
			KeepWorld,
				KeepLocal
		};

		GENERATED_BODY();

	public:

		D_MATH::TransformComponent*			GetTransform() const;

		void								SetParent(GameObject* newParent, AttachmentType attachmentType);

		void								SetActive(bool active);
		bool								IsActive() const;
		INLINE bool							IsSelfActive() const { return mActive && !mDeleted; }

		// Object states
		void								Start();
		void								Awake();
		void								OnDestroy();
		void								OnPreDestroy();

		void								VisitComponents(std::function<void(Darius::Scene::ECS::Components::ComponentBase*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException = nullptr) const;
		void								VisitBehaviourComponents(std::function<void(Darius::Scene::ECS::Components::BehaviourComponent*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException = nullptr) const;
		void								VisitAncestors(std::function<void(GameObject*)> callback) const;
		void								VisitChildren(std::function<void(GameObject*)> callback) const;
		void								VisitDescendants(std::function<void(GameObject*)> callback) const;
		UINT								CountChildren();
		INLINE bool							IsValid() const { return !mDeleted && mEntity.is_valid(); }

		// Copyable Interface
#if _D_EDITOR
		virtual void Copy(bool maintainContext, D_SERIALIZATION::Json& serialized) const override;
		INLINE virtual bool IsCopyableValid() const override { return IsValid(); }
#endif

		template<class T>
		bool								HasComponent() const
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);
			return mEntity.has<T>();
		}

		bool								HasComponent(D_CORE::StringId const& compName) const;

		template<class T>
		INLINE T*							GetComponent() const
		{
			if (!HasComponent<T>())
				return nullptr;

			return const_cast<T*>(mEntity.get<T>());
		}

		ECS::Components::ComponentBase*		GetComponent(D_CORE::StringId const& compName) const;

		template<class T>
		T* AddComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			PreEntityEdit();
			mEntity.add<T>();
			PostEntityEdit();
			auto ref = const_cast<T*>(mEntity.get<T>());
			D_VERIFY(ref);
			AddComponentRoutine(ref);
			return ref;
		}

		template<class T>
		T* AddComponent(T const& value)
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			PreEntityEdit();
			mEntity.add<T>(value);
			PostEntityEdit();
			auto ref = mEntity.get_ref<T>().get();
			AddComponentRoutine(ref);
			return ref;
		}

		D_CONTAINERS::DVector<Darius::Scene::ECS::Components::ComponentBase*> GetComponents(bool sorted = false) const;

		Darius::Scene::ECS::Components::ComponentBase* AddComponent(D_CORE::StringId const& componentName);

		template<class T>
		void								RemoveComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			if (std::is_same<T, Darius::Math::TransformComponent>::value)
				return;

			RemoveComponentRoutine(mEntity.get_ref<T>());

			mEntity.remove<T>();
		}

		void								RemoveComponent(Darius::Scene::ECS::Components::ComponentBase*);

	public:
		INLINE D_ECS::Entity				GetEntity() const { return mEntity; }
		INLINE bool							IsInScene() const { return mInScene; }

		INLINE D_CORE::StringId const&		GetNameId() const { return mName; }
		INLINE std::string					GetName() const { return mName.string(); }
		INLINE D_CORE::Uuid const&			GetPrefab() const { return mPrefab; }
		INLINE Type							GetType() const { return mType; }
		INLINE D_CORE::Uuid const&			GetUuid() const { return mUuid; }
		INLINE bool							IsDeleted() const { return mDeleted; }
		INLINE bool							IsStarted() const { return mStarted; }
		INLINE bool							IsAwake() const { return mAwake; }
		INLINE GameObject*					GetParent() const { return mParent; }
		bool								CanAttachTo(GameObject const* go) const;
		
		INLINE void							SetNameId(D_CORE::StringId const& name) { mName = name; }
		// Call SetNameId instead
		INLINE void							SetName(std::string str) { SetNameId(D_CORE::StringId(str.c_str())); }
		INLINE void							SetType(Type type) { mType = type; }

#ifdef _D_EDITOR
		bool								DrawDetails(float params[]);
		INLINE bool							IsEditableInDetailsWindow() const { return true; }
		void								OnGizmo() const;
		INLINE virtual std::string			GetDetailedName() const override
		{
			return mName.string();
		}

		INLINE virtual Detailed*				GetDetailedParent() const override { return GetParent(); }
#endif // _EDITOR

		static void							RegisterComponent(D_CORE::StringId const& name, D_CONTAINERS::DVector<std::string>& displayName);
		static void							RegisterBehaviourComponent(D_ECS::EntityId componentId);

#ifdef _D_EDITOR
		struct ComponentAddressNode
		{
			D_CORE::StringId					ComponentName;
			D_CONTAINERS::DMap<std::string, ComponentAddressNode> ChildrenNameMap;
			bool								IsBranch;
		};
#endif

		// Events

		// The returned component is already destroyed but not removed from entity
		D_CORE::Signal<void(GameObject*, Darius::Scene::ECS::Components::ComponentBase*)> OnComponentRemove;
		D_CORE::Signal<void(GameObject*, Darius::Scene::ECS::Components::ComponentBase*)> OnComponentAdd;
		D_CORE::Signal<void(GameObject*)> OnComponentSetChange;
		
	protected:
		virtual bool Release() override;

	private:
		friend class D_SCENE::SceneManager;
		friend class Darius::Scene::ECS::Components::ComponentBase;
		friend class Darius::Core::Memory::PagedAllocator<GameObject, true>;

		template<class T>
		friend class D_CORE::Ref;

		GameObject(D_CORE::Uuid const& uuid, D_ECS::Entity entity, bool inScene = true);

		void								PreEntityEdit();
		void								PostEntityEdit();

		void								AddComponentRoutine(Darius::Scene::ECS::Components::ComponentBase*);
		void								RemoveComponentRoutine(Darius::Scene::ECS::Components::ComponentBase*);

#ifdef _D_EDITOR
		void								DrawComponentNameContext(D_CONTAINERS::DMap<std::string, GameObject::ComponentAddressNode> const& componentNameTree);
#endif // _D_EDITOR

		DField(Serialize)
		bool					mActive;

		DField()
		bool					mStarted;

		DField()
		bool					mAwake;

		DField()
		bool					mDeleted;

		DField()
		GameObject* mParent;

		DField(Serialize)
		const D_CORE::Uuid		mUuid;

		DField(Serialize)
		Type					mType;

		D_CORE::StringId		mName;

		DField(Serialize)
		D_CORE::Uuid			mPrefab;

		DField()
		D_ECS::Entity			mEntity;

		DField()
		const bool				mInScene;

		D_CONTAINERS::DVector<std::string> mToRemove;
		D_CONTAINERS::DVector<std::string> mToAdd;

		// Comp name and display name
#if _D_EDITOR
		static D_CONTAINERS::DMap<std::string, GameObject::ComponentAddressNode> RegisteredComponents;
#endif
		static D_CONTAINERS::DSet<D_ECS::EntityId> RegisteredBehaviours;
		static D_CONTAINERS::DSet<D_CORE::StringId> RegisteredComponentNames;
	};

	D_H_SERIALIZE_ENUM(GameObject::Type, {
		{ GameObject::Type::Static, "Static" },
		{ GameObject::Type::Movable, "Movable" }
		});

}

File_GameObject_GENERATED
