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

		float							mMaxGpu = 0.f;
		float							mMaxCpu = 0.f;
		float							mMaxFrameDelta = 0.f;

		float							mLastGpu = 0.f;
		float							mLastCpu = 0.f;
		float							mLastFrameDelta = 0.f;

		float							mAvgGpu = 0.f;
		float							mAvgCpu = 0.f;
		float							mAvgFrameDelta = 0.f;

		float							mHistory = 5.f;
	};

}