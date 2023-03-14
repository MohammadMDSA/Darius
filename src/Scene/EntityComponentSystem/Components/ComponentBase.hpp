#pragma once

#include "Scene/EntityComponentSystem/CompRef.hpp"
#include "Scene/GameObject.hpp"
#include "Scene/Scene.hpp"

#include <Core/Uuid.hpp>
#include <Core/Signal.hpp>
#include <Core/Serialization/Json.hpp>
#include <Math/VectorMath.hpp>
#include <Utils/Common.hpp>
#include <Utils/Log.hpp>
#include <Utils/Assert.hpp>
#include <Utils/StaticConstructor.hpp>

#include <boost/algorithm/string.hpp>

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

#define D_H_COMP_RESOURCE_REF_PROP(type, name, ...) \
public: \
    INLINE void Set##name(D_RESOURCE::ResourceHandle handle) { mChangeSignal(); _Set##name(handle); } \
    INLINE type* Get##name() { return m##name.Get(); } \
    /*INLINE type const* Get##name() const { return m##name.Get(); } */ \
private: \
    INLINE void _Set##name(D_RESOURCE::ResourceHandle handle) { m##name##Handle = handle; m##name = D_RESOURCE::GetResource<type>(handle, *this); __VA_ARGS__ } \
    D_CH_FIELD(D_CORE::Ref<type>, name); \
    D_CH_FIELD(D_RESOURCE::ResourceHandle, name##Handle);

namespace Darius::Scene
{
    class SceneManager;
    class GameObject;
}

namespace Darius::Scene::ECS::Components
{
    class ComponentBase
    {
    public:

        ComponentBase();
        ComponentBase(D_CORE::Uuid uuid);
        ~ComponentBase() { OnDestroy(); }

#ifdef _D_EDITOR
        virtual INLINE bool         DrawDetails(float[]) { return false; }
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

        INLINE bool                 IsActive() const { return mGameObject->GetActive() && mEnabled; }

        // Serialization
        virtual INLINE void         Serialize(D_SERIALIZATION::Json&) const {};
        virtual INLINE void         Deserialize(D_SERIALIZATION::Json const&) {};

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
        INLINE D_MATH::Transform const GetTransform() const { return mGameObject->GetTransform(); }
        INLINE void                 SetLocalTransform(D_MATH::Transform const& transform) { return mGameObject->SetLocalTransform(transform); }
        INLINE void                 SetTransform(D_MATH::Transform const& transform) { return mGameObject->SetTransform(transform); }

        INLINE operator CountedOwner const() {
            auto strName = GetComponentName();
            return CountedOwner{ WSTR_STR(strName), "Game Object Component", this, 0};
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

        // TODO: Add awake status
        D_CH_R_FIELD(D_CORE::Uuid, Uuid);
        D_CH_R_FIELD(Darius::Scene::GameObject*, GameObject);
        D_CH_R_FIELD(bool, Started);
        D_CH_R_FIELD(bool, Enabled);
        D_CH_R_FIELD(bool, Destroyed);

    private:
        friend class Darius::Scene::GameObject;
        friend class Darius::Scene::SceneManager;

        static bool                 sInit;
        static std::string          DisplayName;
	};
}
