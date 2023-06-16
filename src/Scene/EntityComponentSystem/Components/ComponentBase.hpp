#pragma once

#include "Scene/EntityComponentSystem/CompRef.hpp"
#include "Scene/GameObject.hpp"
#include "Scene/Scene.hpp"

#include <Core/Serialization/Json.hpp>
#include <Core/Signal.hpp>
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

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

#define D_H_COMP_BODY(type, parent, compName, shouldRegister) D_H_COMP_BODY_RAW(type, parent, compName, shouldRegister, false, false)

#define D_H_COMP_BODY_RAW(T, parent, compName, shouldRegister, isBehaviour, receivesUpdates) \
public: \
T(); \
T(D_CORE::Uuid uuid); \
static INLINE std::string ClassName() { return D_NAMEOF(T); } \
virtual INLINE std::string GetDisplayName() const override { return T::DisplayName; } \
virtual INLINE std::string GetComponentName() const override { return D_NAMEOF(T); } \
virtual INLINE rttr::type GetComponentType() const override { return rttr::type::get<T>(); }; \
INLINE operator D_ECS::CompRef<T>() { return GetGameObject()->GetComponentRef<T>(); } \
static void StaticConstructor() \
{ \
    /* Registering component*/ \
    if(sInit) \
        return; \
    D_LOG_INFO("Registering " << D_NAMEOF(T) << " child of " << D_NAMEOF(parent)); \
    parent::StaticConstructor(); \
    auto& reg = D_WORLD::GetRegistry(); \
    auto comp = reg.component<T>(D_NAMEOF(T)); \
    auto parentComp = reg.component<parent>(); \
    D_ASSERT(reg.is_valid(parentComp)); \
    comp.is_a(parentComp); \
    D_CONTAINERS::DVector<std::string> splitted; \
    boost::split(splitted, compName, boost::is_any_of("/")); \
    T::DisplayName = splitted[splitted.size() - 1]; \
    if(shouldRegister) \
        D_SCENE::GameObject::RegisterComponent(D_NAMEOF(T), splitted); \
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
D_CORE::Signal<void()> mChangeSignal; \
static bool sInit; \
static std::string DisplayName;

#define D_H_COMP_DEF(type) \
bool type::sInit = false; \
std::string type::DisplayName = "";
//INVOKE_STATIC_CONSTRUCTOR(type);

#define D_H_COMP_DEFAULT_CONSTRUCTOR_DEF_PAR(type, parent) \
type::type() : \
    parent() {} \
type::type(D_CORE::Uuid uuid) : \
    parent(uuid) {}

#define D_H_COMP_DEFAULT_CONSTRUCTOR_DEF(type) D_H_COMP_DEFAULT_CONSTRUCTOR_DEF_PAR(type, ComponentBase)

namespace Darius::Scene
{
    class SceneManager;
}

namespace Darius::Scene::ECS::Components
{
    class DClass(Serialize) ComponentBase
    {
    public:

        ComponentBase();
        ComponentBase(D_CORE::Uuid uuid);
        ~ComponentBase() { OnDestroy(); }

#ifdef _D_EDITOR
        virtual INLINE bool         DrawDetails(float[]) { return false; }
        virtual INLINE void         OnGizmo() const { }
#endif
        virtual INLINE std::string  GetComponentName() const { return ""; }
        virtual INLINE std::string  GetDisplayName() const { return ""; }
        virtual INLINE rttr::type   GetComponentType() const { return rttr::type::get<ComponentBase>(); };

        virtual INLINE void         Start() { }
        virtual INLINE void         Awake() { }
        virtual INLINE void         OnDestroy() { }
        virtual INLINE void         OnSerialized() const { }
        virtual INLINE void         OnDeserialized() { }

        // Gameobject Events
        virtual INLINE void         OnActivate() {}
        virtual INLINE void         OnDeactivate() {}

        virtual INLINE void         Update(float) { };
        virtual INLINE void         LateUpdate(float) { };

        INLINE bool                 IsActive() const { return mGameObject->IsActive() && mGameObject->GetInScene() && mEnabled; }

        virtual INLINE bool         IsDisableable() const { return true; }
        
        virtual INLINE void         SetEnabled(bool value)
        {
            if (!value && !IsDisableable())
                return;

            auto changed = mEnabled != value;
            mEnabled = value;
            if (!changed)
                return;
            if (value)
                OnActivate();
            else
                OnDeactivate();


            if (!mStarted && IsActive())
            {
                mStarted = true;
                Start();
            }

        }

        INLINE CompRef<D_MATH::TransformComponent> GetTransform() const { return mGameObject->GetTransform(); }

        INLINE virtual D_CORE::CountedOwner GetAsCountedOwner()
        {
            auto strName = GetComponentName();
            return D_CORE::CountedOwner{ STR2WSTR(strName), GetComponentType(), this, 0 };
        }

        INLINE operator D_CORE::CountedOwner()
        {
            return GetAsCountedOwner();
        }

        static INLINE std::string   GetName() { return "ComponentBase"; }

        static void                 StaticConstructor()
        {
            if (sInit)
                return;

            auto& reg = D_WORLD::GetRegistry();

            auto comp = reg.component<ComponentBase>("ComponentBase");
            sInit = true;
        }

        static void                 ComponentUpdater(float, D_ECS::ECSRegistry&) {  }
        static void                 ComponentLateUpdater(float, D_ECS::ECSRegistry&) {  }

        static void                 StaticDestructor()
        {
        }


    private:
        friend class Darius::Scene::GameObject;
        friend class Darius::Scene::SceneManager;


        DField(Get[const, &, inline], Serialize)
        D_CORE::Uuid                mUuid;
        
        DField(Get[inline])
        Darius::Scene::GameObject*  mGameObject;
        
        DField(Get[inline])
        bool                        mStarted;
        
        DField(Get[inline], Serialize)
        bool                        mEnabled;
        
        DField(Get[inline])
        bool                        mDestroyed;

        static bool                 sInit;
        static std::string          DisplayName;

        public:
            Darius_Scene_ECS_Components_ComponentBase_GENERATED

	};
}

File_ComponentBase_GENERATED