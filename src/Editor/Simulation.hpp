#pragma once

#ifndef D_SIMULATE
#define D_SIMULATE Darius::Editor::Simulate
#endif // !D_SIMULATE

namespace Darius::Editor::Simulate
{

	void Initialize();
	void Shutdown();

	void Update(float elapsedTime);

	void Run();
	void Pause();
	void Step();

	bool IsSimulating();
}
