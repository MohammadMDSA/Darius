#pragma once

#include <Utils/Common.hpp>

#ifdef _D_EDITOR

#ifndef D_SCENE_DET_DRAW
#define D_SCENE_DET_DRAW Darius::Scene::Utils::DetailsDrawer
#endif // !D_SCENE_DET_DRAW

namespace Darius::Scene::Utils::DetailsDrawer
{
	
	template<typename T>
	INLINE bool DrawDetails(T& elem, float params[])
	{
		return elem.DrawDetails(params);
	}

}

#endif