#pragma once

#include "Window.hpp"
#include "Editor/GUI/Utils/Buffers.hpp"

#include <Core/TimeManager/TimeManager.hpp>

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

		INLINE void ResetGraphs()
		{
			mAvgFrameDelta = mLastFrameDelta = mMaxFrameDelta = D_TIME::GetTargetElapsedSeconds() * 1000;

			mMaxGpu = 0.f;
			mMaxCpu = 0.f;

			mLastGpu = 0.f;
			mLastCpu = 0.f;

			mAvgGpu = 0.f;
			mAvgCpu = 0.f;
			
			mGpuRealtime.Erase();
			mCpuRealtime.Erase();
			mFrameTimeRealtime.Erase();

			mGpuRealtime.AddPoint(0.f, 0.f);
			mCpuRealtime.AddPoint(0.f, 0.f);
			mFrameTimeRealtime.AddPoint(0.f, 0.f);
		}

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

		D_CONTAINERS::DVector<D_PROFILING::ScopeTimeData>mSnapshot;
	};

}