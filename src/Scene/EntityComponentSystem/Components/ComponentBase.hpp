#pragma once

#include "Scene/Scene.hpp"

#include <Core/Uuid.hpp>
#include <Core/Signal.hpp>
#include <Utils/Common.hpp>
#include <Utils/Log.hpp>
#include <Utils/StaticConstructor.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

#define D_H_COMP_BODY(type, parent) \
public: \
type(); \
type(D_CORE::Uuid uuid); \
static INLINE std::string GetName() { return D_NAMEOF(type); } \
virtual INLINE std::string GetComponentName() override { return D_NAMEOF(type); } \
static void StaticConstructor() \
{ \
    if(sInit) \
        return; \
    D_LOG_INFO("Registering" << D_NAMEOF(type) << " child of " << D_NAMEOF(parent)); \
    parent::StaticConstructor(); \
    auto& reg = D_WORLD::GetRegistry(); \
    auto comp = reg.component<type>(D_NAMEOF(type)); \
    comp.add<ComponentTag>(); \
    comp.is_a(reg.component(D_NAMEOF(parent)));\
    sInit = true; \
}\
\
static void StaticDistructor()\
{} \
private: \
static bool sInit;

#define D_H_COMP_DEF(type) \
bool type::sInit = false; \
INVOKE_STATIC_CONSTRUCTOR(type);

namespace Darius::Scene
{
    class SceneManager;
    class GameObject;
}

namespace Darius::Scene::ECS::Components
{
    struct ComponentTag {};

	class ComponentBase
	{
    public:

        ComponentBase();
        ComponentBase(D_CORE::Uuid uuid);

#ifdef _D_EDITOR
        virtual INLINE bool         DrawDetails(float[]) { return false; }
#endif
        virtual INLINE std::string  GetComponentName() { return ""; }

        virtual INLINE void         Start() { }

        static INLINE std::string   GetName() { return "ComponentBase"; }

        static void                 StaticConstructor()
        {
            if (sInit)
                return;

            auto& reg = D_WORLD::GetRegistry();

            auto comp = reg.component<ComponentBase>("ComponentBase");
            comp.add<ComponentTag>();
            
            sInit = true;
        }

        static void                 StaticDestructor()
        {
        }

        D_CH_R_FIELD(D_CORE::Uuid, Uuid);
        D_CH_R_FIELD(Darius::Scene::GameObject*, GameObject);
        D_CH_R_FIELD(bool, Started);

    private:
        friend class Darius::Scene::GameObject;
        friend class Darius::Scene::SceneManager;

        static bool                 sInit;
	};
}
