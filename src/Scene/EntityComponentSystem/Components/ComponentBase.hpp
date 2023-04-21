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

#include <ComponentBase.generated.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

#define D_H_COMP_BODY(type, parent, compName, shouldRegister) D_H_COMP_BODY_RAW(type, parent, compName, shouldRegister, false, false)

#define D_H_COMP_BODY_RAW(type, parent, compName, shouldRegister, isBehaviour, receivesUpdates) \
public: \
type(); \
type(D_CORE::Uuid uuid); \
static INLINE std::string ClassName() { return D_NAMEOF(type); } \
virtual INLINE std::string GetDisplayName() const override { return type::DisplayName; } \
virtual INLINE std::string GetComponentName() const override { return D_NAMEOF(type); } \
INLINE operator D_ECS::CompRef<type>() { return GetGameObject()->GetComponentRef<type>(); } \
static void StaticConstructor() \
{ \
    /* Registering component*/ \
    if(sInit) \
        return; \
    D_LOG_INFO("Registering " << D_NAMEOF(type) << " child of " << D_NAMEOF(parent)); \
    parent::StaticConstructor(); \
    auto& reg = D_WORLD::GetRegistry(); \
    auto comp = reg.component<type>(D_NAMEOF(type)); \
    auto parentComp = reg.component<parent>(); \
    D_ASSERT(reg.is_valid(parentComp)); \
    comp.is_a(parentComp); \
    D_CONTAINERS::DVector<std::string> splitted; \
    boost::split(splitted, compName, boost::is_any_of("/")); \
    type::DisplayName = splitted[splitted.size() - 1]; \
    if(shouldRegister) \
        D_SCENE::GameObject::RegisterComponent(D_NAMEOF(type), splitted); \
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

        virtual INLINE void         Start() { }
        virtual INLINE void         Awake() { }
        virtual INLINE void         OnDestroy() { }

        // Gameobject Events
        virtual INLINE void         OnActivate() {}
        virtual INLINE void         OnDeactivate() {}

        virtual INLINE void         Update(float) { };
        virtual INLINE void         LateUpdate(float) { };

        INLINE bool                 IsActive() const { return mGameObject->IsActive() && mEnabled; }

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
        }

        INLINE D_MATH::Transform const& GetLocalTransform() const { return mGameObject->GetLocalTransform(); }
        INLINE D_MATH::Transform    GetTransform() const { return mGameObject->GetTransform(); }
        INLINE void                 SetLocalTransform(D_MATH::Transform const& transform) { return mGameObject->SetLocalTransform(transform); }
        INLINE void                 SetTransform(D_MATH::Transform const& transform) { return mGameObject->SetTransform(transform); }

        INLINE operator D_CORE::CountedOwner const() {
            auto strName = GetComponentName();
            return D_CORE::CountedOwner { STR2WSTR(strName), "Game Object Component", this, 0};
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
        
        DField(Get[inline])
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