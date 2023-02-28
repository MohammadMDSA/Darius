#pragma once

#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{
	void							Initialize();
	void							Shutdown();

#ifdef _D_EDITOR
	bool							OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

	void							Update(float dt);
}
