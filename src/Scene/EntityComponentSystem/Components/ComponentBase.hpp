#pragma once

#include "Scene/Scene.hpp"
#include "Scene/GameObject.hpp"

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

#define D_H_COMP_BODY(type, parent, compName, shouldRegister, isBehaviour) \
public: \
type(); \
type(D_CORE::Uuid uuid); \
static INLINE std::string GetName() { return D_NAMEOF(type); } \
virtual INLINE std::string GetDisplayName() const override { return type::DisplayName; } \
virtual INLINE std::string GetComponentName() const override { return D_NAMEOF(type); } \
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
        D_SCENE::GameObject::RegisterBehaviourComponent(comp); \
    sInit = true; \
} \
\
static void StaticDistructor() \
{} \
private: \
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

#ifdef _D_EDITOR
#define D_H_RESOURCE_SELECTION_DRAW(resourceType, prop, placeHolder, handleFunction) \
{ \
    resourceType* currentResource = prop.Get(); \
     \
    if (ImGui::Button(placeHolder)) \
    { \
        ImGui::OpenPopup(placeHolder " Res"); \
    } \
     \
    if (ImGui::BeginPopup(placeHolder " Res")) \
    { \
        auto resources = D_RESOURCE::GetResourcePreviews(resourceType::GetResourceType()); \
        int idx = 0; \
        for (auto prev : resources) \
        { \
            bool selected = currentResource && prev.Handle.Id == currentResource->GetId() && prev.Handle.Type == currentResource->GetType(); \
     \
            auto Name = STR_WSTR(prev.Name); \
            ImGui::PushID((Name + std::to_string(idx)).c_str()); \
            if (ImGui::Selectable(Name.c_str(), &selected)) \
            { \
                handleFunction(prev.Handle); \
                changeValue = true; \
            } \
            ImGui::PopID(); \
     \
            idx++; \
        } \
     \
        ImGui::EndPopup(); \
    } \
}
#endif // _D_EDITOR


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

        static INLINE std::string   GetName() { return "ComponentBase"; }

        static void                 StaticConstructor()
        {
            if (sInit)
                return;

            auto& reg = D_WORLD::GetRegistry();

            auto comp = reg.component<ComponentBase>("ComponentBase");
            sInit = true;
        }

        static void                 StaticDestructor()
        {
        }

        D_CH_R_FIELD(D_CORE::Uuid, Uuid);
        D_CH_R_FIELD(Darius::Scene::GameObject*, GameObject);
        D_CH_R_FIELD(bool, Started);
        D_CH_R_FIELD(bool, Enabled);

    private:
        friend class Darius::Scene::GameObject;
        friend class Darius::Scene::SceneManager;

        static bool                 sInit;
        static std::string          DisplayName;
	};
}
