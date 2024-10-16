#include "Editor/pch.hpp"
#include "ProfilerWindow.hpp"

#include "Editor/Simulation.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>

#include <imgui.h>
#include <implot.h>
#include <imgui_widget_flamegraph.h>

#include <Libs/FontIcon/IconsFontAwesome6.h>

namespace Darius::Editor::Gui::Windows
{

	ProfilerWindow::ProfilerWindow(D_SERIALIZATION::Json& config) :
		Window(config)
	{
		D_SIMULATE::SubscribeOnStop([&]()
			{
				ResetGraphs();
			});

		ResetGraphs();
	}

	ProfilerWindow::~ProfilerWindow()
	{
	}

	void ProfilerWindow::Update(float)
	{
		if (!D_SIMULATE::IsSimulating())
			return;

		float time = (float)D_TIME::GetStepTimer()->GetTotalSeconds();

		mMaxGpu = D_PROFILING::GetMaxGpuTime();
		mMaxCpu = D_PROFILING::GetMaxCpuTime();
		mMaxFrameDelta = D_PROFILING::GetMaxFrameDelta();

		mLastGpu = D_PROFILING::GetLastGpuTime();
		mLastCpu = D_PROFILING::GetLastCpuTime();
		mLastFrameDelta = D_PROFILING::GetLastFrameDelta();

		mAvgGpu = D_PROFILING::GetAvgGpuTime();
		mAvgCpu = D_PROFILING::GetAvgCpuTime();
		mAvgFrameDelta = D_PROFILING::GetAvgFrameDelta();

		mGpuRealtime.AddPoint(time, mLastGpu);
		mCpuRealtime.AddPoint(time, mLastCpu);
		mFrameTimeRealtime.AddPoint(time, mLastFrameDelta);
	}

	void ProfilerWindow::DrawGUI()
	{
		D_PROFILING::ScopedTimer profiling(L"Profiler Window Draw GUI");

		if (!ImGui::BeginTabBar("##profilerTabs", ImGuiTabBarFlags_None))
			return;

		auto windowWidth = ImGui::GetWindowWidth();

		ImGui::SetCursorPos({ windowWidth - 50, 25 });
		if (ImGui::Button(ICON_FA_CAMERA))
		{
			mSnapshot.clear();
			D_PROFILING::ScopeTimerSnapshot(mSnapshot);
		}

		// Profiler and Time tab
		if (ImGui::BeginTabItem("Realtime"))
		{

			ImPlotAxisFlags flags = ImPlotAxisFlags_None;
			auto time = D_TIME::GetTotalTime();

			ImGuiTableFlags tableFlags = ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_Resizable;
			ImGuiTableColumnFlags colFlags = ImGuiTableFlags_SizingFixedFit;
			if (ImGui::BeginTable("ProfilerLayout", 2, tableFlags, ImVec2(-1, 160)))
			{
				ImGui::TableSetupColumn("##Metrics", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("##Graph");

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				if (ImGui::BeginTable("ProfilerTimeTable", 5, ImGuiTableFlags_Resizable, ImVec2(300, 0)))
				{
					ImGui::TableSetupColumn("##timeType", colFlags);
					ImGui::TableSetupColumn("CPU", colFlags);
					ImGui::TableSetupColumn("GPU", colFlags);
					ImGui::TableSetupColumn("Frame", colFlags);
					ImGui::TableSetupColumn("FPS", colFlags);

					ImGui::TableHeadersRow();
					ImGui::TableNextRow();

					// Row last
					ImGui::TableNextColumn();
					ImGui::Text("Last");
					// Col CPU
					ImGui::TableNextColumn();
					SetupMetricColor(mLastCpu);
					ImGui::Text("%.2f", mLastCpu);
					ImGui::PopStyleColor();
					// Col GPU
					ImGui::TableNextColumn();
					SetupMetricColor(mLastGpu);
					ImGui::Text("%.2f", mLastGpu);
					ImGui::PopStyleColor();
					// Col Frame
					ImGui::TableNextColumn();
					SetupMetricColor(mLastFrameDelta);
					ImGui::Text("%.2f", mLastFrameDelta);
					// Col FPS
					ImGui::TableNextColumn();
					ImGui::Text("%d", (int)(1000 / mLastFrameDelta));
					ImGui::PopStyleColor();

					ImGui::TableNextRow();

					//Row Avg
					ImGui::TableNextColumn();
					ImGui::Text("Avg");
					// Col CPU
					ImGui::TableNextColumn();
					SetupMetricColor(mAvgCpu);
					ImGui::Text("%.2f", mAvgCpu);
					ImGui::PopStyleColor();
					// Col GPU
					ImGui::TableNextColumn();
					SetupMetricColor(mAvgGpu);
					ImGui::Text("%.2f", mAvgGpu);
					ImGui::PopStyleColor();
					// Col Frame
					ImGui::TableNextColumn();
					SetupMetricColor(mAvgFrameDelta);
					ImGui::Text("%.2f", mAvgFrameDelta);
					// Col FPS
					ImGui::TableNextColumn();
					ImGui::Text("%d", (int)(1000 / mAvgFrameDelta));
					ImGui::PopStyleColor();

					ImGui::TableNextRow();

					// Row Max
					ImGui::TableNextColumn();
					ImGui::Text("Max");
					// Col CPU
					ImGui::TableNextColumn();
					SetupMetricColor(mMaxCpu);
					ImGui::Text("%.2f", mMaxCpu);
					ImGui::PopStyleColor();
					// Col GPU
					ImGui::TableNextColumn();
					SetupMetricColor(mMaxGpu);
					ImGui::Text("%.2f", mMaxGpu);
					ImGui::PopStyleColor();
					// Col Frame
					ImGui::TableNextColumn();
					SetupMetricColor(mMaxFrameDelta);
					ImGui::Text("%.2f", mMaxFrameDelta);
					// Col FPS
					ImGui::TableNextColumn();
					ImGui::Text("%d", (int)(1000 / mMaxFrameDelta));
					ImGui::PopStyleColor();


					ImGui::EndTable();
				}

				ImGui::TableSetColumnIndex(1);
				if (ImPlot::BeginPlot("##ProfilerPlot", ImVec2(-1, -1)))
				{
					// Setup axes
					ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
					ImPlot::SetupAxisLimits(ImAxis_X1, time - mHistory, time, ImGuiCond_Always);
					ImPlot::SetupAxisLimits(ImAxis_Y1, 0, std::max(mMaxGpu, std::max(mMaxCpu, mMaxFrameDelta)), ImGuiCond_Always);

					ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);

					// Showing frame time (shaded)
					ImPlot::PlotShaded("Frame Time", &mFrameTimeRealtime.Data[0].x, &mFrameTimeRealtime.Data[0].y, mFrameTimeRealtime.Data.size(), -INFINITY, 0, mFrameTimeRealtime.Offset, 2 * sizeof(float));

					// Showing cpu time
					ImPlot::PlotLine("CPU Time", &mCpuRealtime.Data[0].x, &mCpuRealtime.Data[0].y, mCpuRealtime.Data.size(), 0, mCpuRealtime.Offset, 2 * sizeof(float));

					// Showing gpu time
					ImPlot::PlotLine("GPU Time", &mGpuRealtime.Data[0].x, &mGpuRealtime.Data[0].y, mGpuRealtime.Data.size(), 0, mGpuRealtime.Offset, 2 * sizeof(float));

					ImPlot::EndPlot();
				}
				ImGui::EndTable();

			}

			ImGui::EndTabItem();
		}
		// Flame Graph Tab
		if (ImGui::BeginTabItem("Snapshot Flame Graph"))
		{

			ImGuiWidgetFlameGraph::PlotFlame(L"", &D_PROFILING::CpuProfilerValueGetter, mSnapshot.data(), (int)mSnapshot.size(), 1, L"", FLT_MAX, FLT_MAX, { windowWidth - 10, 0.f });

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	void ProfilerWindow::SetupMetricColor(float val, float bad, float warn)
	{
		if (val >= bad)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 20, 20, 255));
		else if (val >= warn)
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 20, 255));
		else
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(20, 255, 20, 255));
	}

}
