#pragma once

#include "EntityComponentSystem/Entity.hpp"
#include "EntityComponentSystem/CompRef.hpp"

#include <Core/Ref.hpp>
#include <Core/Uuid.hpp>
#include <Core/Serialization/Json.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Core/Containers/Set.hpp>
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
	class GameObject;
	
	class DClass(Serialize) GameObject : public Detailed
	{
	public:

		enum class Type
		{
			Static,
			Movable
		};

	public:

		// Transform helpers
		void								SetLocalTransform(D_MATH::Transform const& trans);
		D_MATH::Transform const&			GetLocalTransform() const;

		INLINE void							SetTransform(D_MATH::Transform const& trans)
		{
			if (mType == Type::Static)
				return;
			if (mParent)
				SetLocalTransform((DirectX::XMMATRIX)(trans.GetWorld() * D_MATH::Matrix4(mParent->GetTransform()).Inverse()));
			else
				SetLocalTransform(trans);
		}

		INLINE D_MATH::Transform			GetTransform() const
		{
			if (mParent)
				return GetLocalTransform() * mParent->GetTransform();
			return GetLocalTransform();
		}

		void								SetParent(GameObject* newParent);

		void								SetActive(bool active);

		// Object states
		void								Start();
		void								Awake();
		void								OnDestroy();

		void								VisitComponents(std::function<void(Darius::Scene::ECS::Components::ComponentBase*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException = nullptr) const;
		void								VisitBehaviourComponents(std::function<void(Darius::Scene::ECS::Components::BehaviourComponent*)> callback, std::function<void(D_EXCEPTION::Exception const&)> onException = nullptr) const;
		void								VisitAncestors(std::function<void(GameObject*)> callback) const;
		void								VisitChildren(std::function<void(GameObject*)> callback) const;
		void								VisitDescendants(std::function<void(GameObject*)> callback) const;
		UINT								CountChildren();
		INLINE bool							IsValid() const { return !mDeleted && mEntity.is_valid(); }

		template<class T>
		T* GetComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);
			return mEntity.get_ref<T>().get();
		}

		template<class T>
		bool								HasComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);
			return mEntity.has<T>();
		}

		template<class T>
		T*									AddComponent()
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			mEntity.add<T>();
			auto ref = mEntity.get_ref<T>().get();
			AddComponentRoutine(ref);
			return ref;
		}

		template<class T>
		T*									AddComponent(T const& value)
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			mEntity.add<T>(value);
			auto ref = mEntity.get_ref<T>().get();
			AddComponentRoutine(ref);
			return ref;
		}

		template<class T>
		INLINE D_ECS::CompRef<T>			GetComponentRef()
		{
			return D_ECS::CompRef<T>(mEntity);
		}

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


#ifdef _D_EDITOR
		bool								DrawDetails(float params[]);
		void								OnGizmo() const;
#endif // _EDITOR

		INLINE operator D_CORE::CountedOwner const() {
			return D_CORE::CountedOwner { STR2WSTR(mName), rttr::type::get<GameObject>(), this, 0};
		}

		static void							RegisterComponent(std::string name, D_CONTAINERS::DVector<std::string>& displayName);
		static void							RegisterBehaviourComponent(D_ECS::EntityId componentId);

		struct ComponentAddressNode
		{
			std::string							ComponentName;
			D_CONTAINERS::DMap<std::string, ComponentAddressNode> ChildrenNameMap;
			bool								IsBranch;
		};

	private:
		friend class D_SCENE::SceneManager;
		friend class Darius::Scene::ECS::Components::ComponentBase;

		friend void							to_json(D_SERIALIZATION::Json& j, const GameObject& value);
		friend void							from_json(const D_SERIALIZATION::Json& j, GameObject& value);

		GameObject(D_CORE::Uuid uuid, D_ECS::Entity entity);

		void								AddComponentRoutine(Darius::Scene::ECS::Components::ComponentBase*);
		void								RemoveComponentRoutine(Darius::Scene::ECS::Components::ComponentBase*);

#ifdef _D_EDITOR
		void								DrawComponentNameContext(D_CONTAINERS::DMap<std::string, GameObject::ComponentAddressNode> const& componentNameTree);
#endif // _D_EDITOR

		DField(Get[inline])
		bool					mActive;
		
		DField(Get[inline])
		bool					mStarted;
		
		DField(Get[inline])
		bool					mAwake;
		
		DField(Get[inline])
		bool					mDeleted;

		DField(Get[inline])
		GameObject*				mParent;

		DField(Get[const, &, inline])
		const D_CORE::Uuid		mUuid;

		DField(Get[inline], Set[inline])
		Type					mType;
		
		DField(Get[inline, const, &], Set[inline])
		std::string				mName;

		D_ECS::Entity			mEntity;

		// Comp name and display name
		static D_CONTAINERS::DMap<std::string, GameObject::ComponentAddressNode> RegisteredComponents;
		static D_CONTAINERS::DSet<D_ECS::EntityId> RegisteredBehaviours;
		static D_CONTAINERS::DSet<std::string> RegisteredComponentNames;

		public:
			Darius_Scene_GameObject_GENERATED
	};

	D_H_SERIALIZE_ENUM(GameObject::Type, {
		{ GameObject::Type::Static, "Static" },
		{ GameObject::Type::Movable, "Movable" }
		});

	void to_json(D_SERIALIZATION::Json& j, const GameObject& value);

	void from_json(const D_SERIALIZATION::Json& j, GameObject& value);
}

File_GameObject_GENERATED
