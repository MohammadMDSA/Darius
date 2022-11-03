#pragma once

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{
	void							Initialize();
	void							Shutdown();

	void							Update(float dt);
}
