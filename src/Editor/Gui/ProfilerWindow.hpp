#pragma once

#include "Window.hpp"
#include "Utils/Buffers.hpp"

namespace Darius::Editor::Gui::Windows
{
	class ProfilerWindow : public Window
	{
	public:
		ProfilerWindow();
		~ProfilerWindow();

		// Inherited via Window
		INLINE virtual std::string const GetName() override { return "Profiler"; }

		INLINE virtual void Render(D_GRAPHICS::GraphicsContext&) override {}

		virtual void Update(float) override;

		virtual void DrawGUI() override;

	private:

		void SetupMetricColor(float val, float bad = 33.333f, float warn = 22.222f);

		Utils::ScrollingBuffer			mGpuRealtime;
		Utils::ScrollingBuffer			mCpuRealtime;
		Utils::ScrollingBuffer			mFrameTimeRealtime;

		float							mMaxGpu;
		float							mMaxCpu;
		float							mMaxFrameDelta;

		float							mLastGpu;
		float							mLastCpu;
		float							mLastFrameDelta;

		float							mAvgGpu;
		float							mAvgCpu;
		float							mAvgFrameDelta;

		float							mHistory = 5.f;
	};

}