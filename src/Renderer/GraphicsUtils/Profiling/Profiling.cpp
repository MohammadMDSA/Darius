//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "Renderer/pch.hpp"
#include "Profiling.hpp"
#include "GpuTimeManager.hpp"
#include "Renderer/CommandContext.hpp"
#include "Renderer/GraphicsCore.hpp"

#include <Core/TimeManager/SystemTime.hpp>

using namespace std;
using namespace D_GRAPHICS;

namespace Darius::Graphics::Utils::Profiling
{

	bool Paused = false;

	class StatPlot
	{
	public:
		StatPlot(StatHistory& Data, Color Col = Color(1.0f, 1.0f, 1.0f))
			: m_StatData(Data), m_PlotColor(Col)
		{
		}

		void SetColor(Color Col)
		{
			m_PlotColor = Col;
		}

	private:
		StatHistory& m_StatData;
		Color m_PlotColor;
	};

	class StatGraph
	{
	public:
		StatGraph(const wstring& Label, D3D12_RECT Window)
			: m_Label(Label), m_Window(Window), m_BGColor(0.0f, 0.0f, 0.0f, 0.2f)
		{
		}

		void SetLabel(const wstring& Label)
		{
			m_Label = Label;
		}

		void SetWindow(D3D12_RECT Window)
		{
			m_Window = Window;
		}

		uint32_t AddPlot(const StatPlot& P)
		{
			uint32_t Idx = (uint32_t)m_Stats.size();
			m_Stats.push_back(P);
			return Idx;
		}

		StatPlot& GetPlot(uint32_t Handle);

		void Draw(GraphicsContext& Context);

	private:
		wstring m_Label;
		D3D12_RECT m_Window;
		vector<StatPlot> m_Stats;
		Color m_BGColor;
		float m_PeakValue;
	};

	class GraphManager
	{
	public:

	private:
		vector<StatGraph> m_Graphs;
	};

	class GpuTimer
	{
	public:

		GpuTimer()
		{
			m_TimerIndex = D_PROFILING_GPU::NewTimer();
		}

		void Start(CommandContext& Context)
		{
			D_PROFILING_GPU::StartTimer(Context, m_TimerIndex);
		}

		void Stop(CommandContext& Context)
		{
			D_PROFILING_GPU::StopTimer(Context, m_TimerIndex);
		}

		float GetTime(void)
		{
			return D_PROFILING_GPU::GetTime(m_TimerIndex);
		}

		uint32_t GetTimerIndex(void)
		{
			return m_TimerIndex;
		}
	private:

		uint32_t m_TimerIndex;
	};

	class NestedTimingTree
	{
	public:
		NestedTimingTree(const wstring& name, NestedTimingTree* parent = nullptr)
			: m_Name(name), m_Parent(parent), m_IsExpanded(false) {}

		NestedTimingTree* GetChild(const wstring& name)
		{
			auto iter = m_LUT.find(name);
			if (iter != m_LUT.end())
				return iter->second;

			NestedTimingTree* node = new NestedTimingTree(name, this);
			m_Children.push_back(node);
			m_LUT[name] = node;
			return node;
		}

		NestedTimingTree* NextScope(void)
		{
			if (m_IsExpanded && m_Children.size() > 0)
				return m_Children[0];

			return m_Parent->NextChild(this);
		}

		NestedTimingTree* PrevScope(void)
		{
			NestedTimingTree* prev = m_Parent->PrevChild(this);
			return prev == m_Parent ? prev : prev->LastChild();
		}

		NestedTimingTree* FirstChild(void)
		{
			return m_Children.size() == 0 ? nullptr : m_Children[0];
		}

		NestedTimingTree* LastChild(void)
		{
			if (!m_IsExpanded || m_Children.size() == 0)
				return this;

			return m_Children.back()->LastChild();
		}

		NestedTimingTree* NextChild(NestedTimingTree* curChild)
		{
			D_ASSERT(curChild->m_Parent == this);

			for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
			{
				if (*iter == curChild)
				{
					auto nextChild = iter; ++nextChild;
					if (nextChild != m_Children.end())
						return *nextChild;
				}
			}

			if (m_Parent != nullptr)
				return m_Parent->NextChild(this);
			else
				return &sm_RootScope;
		}

		NestedTimingTree* PrevChild(NestedTimingTree* curChild)
		{
			D_ASSERT(curChild->m_Parent == this);

			if (*m_Children.begin() == curChild)
			{
				if (this == &sm_RootScope)
					return sm_RootScope.LastChild();
				else
					return this;
			}

			for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
			{
				if (*iter == curChild)
				{
					auto prevChild = iter; --prevChild;
					return *prevChild;
				}
			}

			ERROR("All attempts to find a previous timing sample failed");
			return nullptr;
		}

		void StartTiming(CommandContext* Context)
		{
			m_StartTick = D_TIME::SystemTime::GetCurrentTick();
			if (Context == nullptr)
				return;

			m_GpuTimer.Start(*Context);

			Context->PIXBeginEvent(m_Name.c_str());
		}

		void StopTiming(CommandContext* Context)
		{
			m_EndTick = D_TIME::SystemTime::GetCurrentTick();
			if (Context == nullptr)
				return;

			m_GpuTimer.Stop(*Context);

			Context->PIXEndEvent();
		}

		void GatherTimes(uint32_t FrameIndex)
		{
			if (sm_SelectedScope == this)
			{
			}
			if (Paused)
			{
				for (auto node : m_Children)
					node->GatherTimes(FrameIndex);
				return;
			}
			m_CpuTime.RecordStat(FrameIndex, 1000.0f * (float)D_TIME::SystemTime::TimeBetweenTicks(m_StartTick, m_EndTick));
			m_GpuTime.RecordStat(FrameIndex, 1000.0f * m_GpuTimer.GetTime());

			for (auto node : m_Children)
				node->GatherTimes(FrameIndex);

			m_StartTick = 0;
			m_EndTick = 0;
		}

		void SumInclusiveTimes(float& cpuTime, float& gpuTime)
		{
			cpuTime = 0.0f;
			gpuTime = 0.0f;
			for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
			{
				cpuTime += (*iter)->m_CpuTime.GetLast();
				gpuTime += (*iter)->m_GpuTime.GetLast();
			}
		}

		static void PushProfilingMarker(const wstring& name, CommandContext* Context);
		static void PopProfilingMarker(CommandContext* Context);
		static void Update(void);
		static void UpdateTimes(void)
		{
			uint32_t FrameIndex = (uint32_t)D_GRAPHICS::GetFrameCount();
			uint64_t frameTick = D_TIME::SystemTime::GetCurrentTick();

			D_PROFILING_GPU::BeginReadBack();
			sm_RootScope.GatherTimes(FrameIndex);
			s_FrameDelta.RecordStat(FrameIndex, D_TIME::SystemTime::TimeBetweenTicks(s_lastFrameTick, frameTick));
			D_PROFILING_GPU::EndReadBack();

			float TotalCpuTime, TotalGpuTime;
			sm_RootScope.SumInclusiveTimes(TotalCpuTime, TotalGpuTime);
			s_TotalCpuTime.RecordStat(FrameIndex, TotalCpuTime);
			s_TotalGpuTime.RecordStat(FrameIndex, TotalGpuTime);

			s_lastFrameTick = frameTick;
		}

		static float GetAvgCpuTime() { return s_TotalCpuTime.GetAvg(); }
		static float GetAvgGpuTime() { return s_TotalGpuTime.GetAvg(); }
		static float GetAvgFrameDelta() { return s_FrameDelta.GetAvg(); }
		static float GetLastCpuTime() { return s_TotalCpuTime.GetLast(); }
		static float GetLastGpuTime() { return s_TotalGpuTime.GetLast(); }
		static float GetLastFrameDelta() { return s_FrameDelta.GetLast(); }
		static float GetMinCpuTime() { return s_TotalCpuTime.GetMin(); }
		static float GetMinGpuTime() { return s_TotalGpuTime.GetMin(); }
		static float GetMinFrameDelta() { return s_FrameDelta.GetMin(); }
		static float GetMaxCpuTime() { return s_TotalCpuTime.GetMax(); }
		static float GetMaxGpuTime() { return s_TotalGpuTime.GetMax(); }
		static float GetMaxFrameDelta() { return s_FrameDelta.GetMax(); }

		static INLINE void SetLastFrameTick(uint64_t tick)
		{
			s_lastFrameTick = tick;
		}

	private:

		void DeleteChildren(void)
		{
			for (auto node : m_Children)
				delete node;
			m_Children.clear();
		}

		wstring m_Name;
		NestedTimingTree* m_Parent;
		vector<NestedTimingTree*> m_Children;
		unordered_map<wstring, NestedTimingTree*> m_LUT;
		int64_t m_StartTick;
		int64_t m_EndTick;
		StatHistory m_CpuTime;
		StatHistory m_GpuTime;
		bool m_IsExpanded;
		GpuTimer m_GpuTimer;
		static StatHistory s_TotalCpuTime;
		static StatHistory s_TotalGpuTime;
		static StatHistory s_FrameDelta;
		static NestedTimingTree sm_RootScope;
		static NestedTimingTree* sm_CurrentNode;
		static NestedTimingTree* sm_SelectedScope;
		static uint64_t s_lastFrameTick;
	};

	StatHistory NestedTimingTree::s_TotalCpuTime;
	StatHistory NestedTimingTree::s_TotalGpuTime;
	StatHistory NestedTimingTree::s_FrameDelta;
	NestedTimingTree NestedTimingTree::sm_RootScope(L"");
	NestedTimingTree* NestedTimingTree::sm_CurrentNode = &NestedTimingTree::sm_RootScope;
	NestedTimingTree* NestedTimingTree::sm_SelectedScope = &NestedTimingTree::sm_RootScope;
	uint64_t NestedTimingTree::s_lastFrameTick = D_TIME::SystemTime::GetCurrentTick();

	const bool DrawPerfGraph = false;

	float GetAvgCpuTime() { return NestedTimingTree::GetAvgCpuTime(); }
	float GetAvgGpuTime() { return NestedTimingTree::GetAvgGpuTime(); }
	float GetAvgFrameDelta() { return NestedTimingTree::GetAvgFrameDelta() * 1000; }
	float GetLastCpuTime() { return NestedTimingTree::GetLastCpuTime(); }
	float GetLastGpuTime() { return NestedTimingTree::GetLastGpuTime(); }
	float GetLastFrameDelta() { return NestedTimingTree::GetLastFrameDelta() * 1000; }
	float GetMinCpuTime() { return NestedTimingTree::GetMinCpuTime(); }
	float GetMinGpuTime() { return NestedTimingTree::GetMinGpuTime(); }
	float GetMinFrameDelta() { return NestedTimingTree::GetMinFrameDelta() * 1000; }
	float GetMaxCpuTime() { return NestedTimingTree::GetMaxCpuTime(); }
	float GetMaxGpuTime() { return NestedTimingTree::GetMaxGpuTime(); }
	float GetMaxFrameDelta() { return NestedTimingTree::GetMaxFrameDelta() * 1000; }
	int GetFrameRate() { return (int)(1 / NestedTimingTree::GetLastFrameDelta()); }

	void Update(void)
	{
		/*if (GameInput::IsFirstPressed(GameInput::kStartButton)
			|| GameInput::IsFirstPressed(GameInput::kKey_space))
		{
			Paused = !Paused;
		}*/
		NestedTimingTree::UpdateTimes();
	}

	void BeginBlock(const wstring& name, CommandContext* Context)
	{
		NestedTimingTree::PushProfilingMarker(name, Context);
	}

	void EndBlock(CommandContext* Context)
	{
		NestedTimingTree::PopProfilingMarker(Context);
	}

	void Pause()
	{
		if (Paused)
			return;
		Paused = true;
	}

	void Resume()
	{
		if (!Paused)
			return;
		Paused = false;
		NestedTimingTree::SetLastFrameTick(D_TIME::SystemTime::GetCurrentTick());
	}

	bool IsPaused()
	{
		return Paused;
	}

	//void DisplayFrameRate(TextContext& Text)
	//{
	//    if (!DrawFrameRate)
	//        return;

	//    float cpuTime = NestedTimingTree::GetTotalCpuTime();
	//    float gpuTime = NestedTimingTree::GetTotalGpuTime();
	//    float frameRate = 1.0f / NestedTimingTree::GetFrameDelta();

	//    Text.DrawFormattedString("CPU %7.3f ms, GPU %7.3f ms, %3u Hz\n",
	//        cpuTime, gpuTime, (uint32_t)(frameRate + 0.5f));
	//}

	//void DisplayPerfGraph(GraphicsContext& Context)
	//{
	//    if (DrawPerfGraph)
	//        GraphRenderer::RenderGraphs(Context, GraphType::Global);
	//}

	//void Display(TextContext& Text, float x, float y, float /*w*/, float /*h*/)
	//{
	//    Text.ResetCursor(x, y);

	//    if (DrawProfiler)
	//    {
	//        //Text.GetCommandContext().SetScissor((uint32_t)Floor(x), (uint32_t)Floor(y), (uint32_t)Ceiling(w), (uint32_t)Ceiling(h));

	//        NestedTimingTree::Update();

	//        Text.SetColor(Color(0.5f, 1.0f, 1.0f));
	//        Text.DrawString("Engine Profiling");
	//        Text.SetColor(Color(0.8f, 0.8f, 0.8f));
	//        Text.SetTextSize(20.0f);
	//        Text.DrawString("           CPU    GPU");
	//        Text.SetTextSize(24.0f);
	//        Text.NewLine();
	//        Text.SetTextSize(20.0f);
	//        Text.SetColor(Color(1.0f, 1.0f, 1.0f));

	//        NestedTimingTree::Display(Text, x);
	//    }

	//    Text.GetCommandContext().SetScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
	//}

	void NestedTimingTree::PushProfilingMarker(const wstring& name, CommandContext* Context)
	{
		sm_CurrentNode = sm_CurrentNode->GetChild(name);
		sm_CurrentNode->StartTiming(Context);
	}

	void NestedTimingTree::PopProfilingMarker(CommandContext* Context)
	{
		sm_CurrentNode->StopTiming(Context);
		sm_CurrentNode = sm_CurrentNode->m_Parent;
	}

	void NestedTimingTree::Update(void)
	{
		D_ASSERT_M(sm_SelectedScope != nullptr, "Corrupted profiling data structure");

		if (sm_SelectedScope == &sm_RootScope)
		{
			sm_SelectedScope = sm_RootScope.FirstChild();
			if (sm_SelectedScope == &sm_RootScope)
				return;
		}

		//if (GameInput::IsFirstPressed(GameInput::kDPadLeft)
		//    || GameInput::IsFirstPressed(GameInput::kKey_left))
		//{
		//    //if still on graphs go back to text
		//    if (sm_CursorOnGraph)
		//        sm_CursorOnGraph = !sm_CursorOnGraph;
		//    else
		//        sm_SelectedScope->m_IsExpanded = false;
		//}
		//else if (GameInput::IsFirstPressed(GameInput::kDPadRight)
		//    || GameInput::IsFirstPressed(GameInput::kKey_right))
		//{
		//    if (sm_SelectedScope->m_IsExpanded == true && !sm_CursorOnGraph)
		//        sm_CursorOnGraph = true;
		//    else
		//        sm_SelectedScope->m_IsExpanded = true;
		//    //if already expanded go over to graphs

		//}
		//else if (GameInput::IsFirstPressed(GameInput::kDPadDown)
		//    || GameInput::IsFirstPressed(GameInput::kKey_down))
		//{
		//    sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->NextScope() : nullptr;
		//}
		//else if (GameInput::IsFirstPressed(GameInput::kDPadUp)
		//    || GameInput::IsFirstPressed(GameInput::kKey_up))
		//{
		//    sm_SelectedScope = sm_SelectedScope ? sm_SelectedScope->PrevScope() : nullptr;
		//}
		//else if (GameInput::IsFirstPressed(GameInput::kAButton)
		//    || GameInput::IsFirstPressed(GameInput::kKey_return))
		//{
		//    sm_SelectedScope->Toggle();
		//}

	}

	//void NestedTimingTree::DisplayNode(TextContext& Text, float leftMargin, float indent)
	//{
	//    if (this == &sm_RootScope)
	//    {
	//        m_IsExpanded = true;
	//        sm_RootScope.FirstChild()->m_IsExpanded = true;
	//    }
	//    else
	//    {
	//        if (sm_SelectedScope == this && !sm_CursorOnGraph)
	//            Text.SetColor(Color(1.0f, 1.0f, 0.5f));
	//        else
	//            Text.SetColor(Color(1.0f, 1.0f, 1.0f));


	//        Text.SetLeftMargin(leftMargin);
	//        Text.SetCursorX(leftMargin);

	//        if (m_Children.size() == 0)
	//            Text.DrawString("  ");
	//        else if (m_IsExpanded)
	//            Text.DrawString("- ");
	//        else
	//            Text.DrawString("+ ");

	//        Text.DrawString(m_Name.c_str());
	//        Text.SetCursorX(leftMargin + 300.0f);
	//        Text.DrawFormattedString("%6.3f %6.3f   ", m_CpuTime.GetAvg(), m_GpuTime.GetAvg());

	//        if (IsGraphed())
	//        {
	//            Text.SetColor(GraphRenderer::GetGraphColor(m_GraphHandle, GraphType::Profile));
	//            Text.DrawString("  []\n");
	//        }
	//        else
	//            Text.DrawString("\n");
	//    }

	//    if (!m_IsExpanded)
	//        return;

	//    for (auto node : m_Children)
	//        node->DisplayNode(Text, leftMargin + indent, indent);
	//}

	//void NestedTimingTree::StoreToGraph(void)
	//{
	//    if (m_GraphHandle != PERF_GRAPH_ERROR)
	//        GraphRenderer::Update(XMFLOAT2(m_CpuTime.GetLast(), m_GpuTime.GetLast()), m_GraphHandle, GraphType::Profile);

	//    for (auto node : m_Children)
	//        node->StoreToGraph();
	//}
}