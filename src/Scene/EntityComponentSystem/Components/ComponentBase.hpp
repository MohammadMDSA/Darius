#pragma once

#include "Scene/EntityComponentSystem/CompRef.hpp"
#include "Scene/EntityComponentSystem/ComponentEvent.hpp"
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
using Super = parent; \
using ThisClass = T; \
static constexpr INLINE std::string ClassName() { return D_NAMEOF(T); } \
static INLINE D_ECS::ComponentEntry GetComponentEntryStatic() { return D_WORLD::GetComponentEntity(ClassName()); } \
virtual INLINE D_ECS::ComponentEntry GetComponentEntry() const override { return D_WORLD::GetComponentEntity(ClassName()); } \
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
    auto comp = D_WORLD::RegisterComponent<T, parent>(); \
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
D_CORE::Signal<void(ComponentBase*)> mChangeSignal; \
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
        GENERATED_BODY();

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
        virtual INLINE ComponentEntry GetComponentEntry() const { return D_WORLD::GetComponentEntity(ClassName()); }


        virtual INLINE void         Start() { }
        virtual INLINE void         Awake() { }
        virtual INLINE void         OnDestroy() { }
        virtual INLINE void         OnPreDestroy() { }
        virtual INLINE void         OnSerialized() const { }
        virtual INLINE void         OnDeserialized() { }

        // GameObject Events
        virtual INLINE void         OnActivate() {}
        virtual INLINE void         OnDeactivate() {}

        virtual INLINE void         Update(float) { };
        virtual INLINE void         LateUpdate(float) { };


        void						SetDirty();
        virtual INLINE bool			IsDirty() const { return mDirty; }
        bool						CanChange() const;

        bool                        IsActive() const;

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


    private:
        friend class Darius::Scene::GameObject;
        friend class Darius::Scene::SceneManager;


        DField(Get[const, &, inline], Serialize, NotAnimate)
        D_CORE::Uuid                mUuid;
        
        DField(Get[inline])
        Darius::Scene::GameObject*  mGameObject;
        
        DField(Get[inline])
        bool                        mStarted;
        
        DField(Get[inline], Serialize)
        bool                        mEnabled;
        
        DField(Get[inline])
        bool                        mDestroyed;

        bool						mDirty;

        static bool                 sInit;
        static std::string          DisplayName;

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

namespace Darius::Scene::ECS
{

    template<typename ...T>
    class GenericComponentSignal : public D_CORE::Signal<void(T...)>
    {
    public:
        typedef GenericComponentSignalSlot<void, T...> Slot;

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
            return this->connect(slot);
        }
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

File_ComponentBase_GENERATED