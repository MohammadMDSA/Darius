#pragma once

#include "EntityComponentSystem/Entity.hpp"

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

namespace Darius::Scene
{
	class SceneManager;

	class DClass(Serialize) GameObject sealed : public Detailed, public D_CORE::Counted, public D_SERIALIZATION::ICopyable
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
		virtual void Copy(bool maintainContext, D_SERIALIZATION::Json& serialized) const override;
		INLINE virtual bool IsCopyableValid() const override { return IsValid(); }

		template<class T>
		bool								HasComponent() const
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);
			return mEntity.has<T>();
		}

		bool								HasComponent(std::string const& compName) const;

		template<class T>
		INLINE T*							GetComponent() const
		{
			if (!HasComponent<T>())
				return nullptr;

			return const_cast<T*>(mEntity.get<T>());
		}

		ECS::Components::ComponentBase*		GetComponent(std::string const& compName) const;

		template<class T>
		T* AddComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			mEntity.add<T>();
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

			mEntity.add<T>(value);
			auto ref = mEntity.get_ref<T>().get();
			AddComponentRoutine(ref);
			return ref;
		}

		D_CONTAINERS::DVector<Darius::Scene::ECS::Components::ComponentBase*> GetComponents(bool sorted = false) const;

		Darius::Scene::ECS::Components::ComponentBase* AddComponent(std::string const& componentName);

		template<class T>
		void								RemoveComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			if (std::is_same<T, Darius::Math::TransformComponent>::value)
				return;

			mEntity.remove<T>();

			RemoveComponentRoutine(mEntity.get_ref<T>());
		}

		void								RemoveComponent(Darius::Scene::ECS::Components::ComponentBase*);

	public:
		INLINE D_ECS::Entity				GetEntity() const { return mEntity; }
		INLINE bool							IsInScene() const { return mInScene; }

		INLINE std::string const&			GetName() const { return mName; }
		INLINE D_CORE::Uuid const&			GetPrefab() const { return mPrefab; }
		INLINE Type							GetType() const { return mType; }
		INLINE D_CORE::Uuid const&			GetUuid() const { return mUuid; }
		INLINE bool							IsDeleted() const { return mDeleted; }
		INLINE bool							IsStarted() const { return mStarted; }
		INLINE bool							IsAwake() const { return mAwake; }
		INLINE GameObject*					GetParent() const { return mParent; }

#ifdef _D_EDITOR
		bool								DrawDetails(float params[]);
		INLINE bool							IsEditableInDetailsWindow() const { return true; }
		void								OnGizmo() const;
#endif // _EDITOR

		static void							RegisterComponent(std::string name, D_CONTAINERS::DVector<std::string>& displayName);
		static void							RegisterBehaviourComponent(D_ECS::EntityId componentId);

#ifdef _D_EDITOR
		struct ComponentAddressNode
		{
			std::string							ComponentName;
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

		template<class T>
		friend class D_CORE::Ref;

		GameObject(D_CORE::Uuid uuid, D_ECS::Entity entity, bool inScene = true);

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

		DField(Set[inline], Serialize)
		Type					mType;

		DField(Set[inline], Serialize)
		std::string				mName;

		DField(Serialize)
		D_CORE::Uuid			mPrefab;

		DField()
		D_ECS::Entity			mEntity;

		DField()
		const bool				mInScene;

		// Comp name and display name
#if _D_EDITOR
		static D_CONTAINERS::DMap<std::string, GameObject::ComponentAddressNode> RegisteredComponents;
#endif
		static D_CONTAINERS::DSet<D_ECS::EntityId> RegisteredBehaviours;
		static D_CONTAINERS::DSet<std::string> RegisteredComponentNames;

	};

	D_H_SERIALIZE_ENUM(GameObject::Type, {
		{ GameObject::Type::Static, "Static" },
		{ GameObject::Type::Movable, "Movable" }
		});

}

File_GameObject_GENERATED
