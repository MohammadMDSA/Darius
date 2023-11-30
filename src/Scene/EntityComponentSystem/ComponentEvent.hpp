#pragma once

#include "CompRef.hpp"

#include <Core/Signal.hpp>

#include <functional>
#include <type_traits>

#ifndef D_ECS
#define D_ECS Darius::Scene::ECS
#endif // !D_ECS

namespace Darius::Scene::ECS
{

	// A signal slot which is suitable for defining any argument type list,
	// and calls a safe reference of a component of an entity
	template<typename R, typename ...T>
	struct GenericComponentSignalSlot
	{
		typedef R							ReturnType;

		UntypedCompRef						Comp;
		std::function<ReturnType(T...)>		Func;

		ReturnType operator() (T... args)
		{
			auto compRef = Comp.Get();
			if(compRef)
				return compRef->*Func(args...);

			return ReturnType();
		}
	};

}
