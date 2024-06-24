#pragma once

#include <functional>

#ifndef D_SIMULATE
#define D_SIMULATE Darius::Editor::Simulate
#endif // !D_SIMULATE

namespace Darius::Editor::Simulate
{

	void Initialize();
	void Shutdown();

	void Update(float editorDeltaTime, std::function<void()> externalContextUpdate = nullptr);

	void Run();
	void Stop();

	void Pause();
	void Resume();

	void Step();

	bool IsSimulating();
	bool IsPaused();
	bool IsStepping();

	void SubscribeOnRun(std::function<void()> callback);
	void SubscribeOnStop(std::function<void()> callback);
}
