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

#include <Core/Containers/Map.hpp>
#include <Core/TimeManager/SystemTime.hpp>

using namespace D_GRAPHICS;
using namespace D_MATH;

namespace Darius::Graphics::Utils::Profiling
{

	bool Paused = false;
	bool AwaitingUpdate = true;

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
		StatGraph(const std::wstring& Label, D3D12_RECT Window)
			: m_Label(Label), m_Window(Window), m_BGColor(0.0f, 0.0f, 0.0f, 0.2f)
		{
		}

		void SetLabel(const std::wstring& Label)
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
		std::wstring m_Label;
		D3D12_RECT m_Window;
		D_CONTAINERS::DVector<StatPlot> m_Stats;
		Color m_BGColor;
		float m_PeakValue;
	};

	class GraphManager
	{
	public:

	private:
		D_CONTAINERS::DVector<StatGraph> m_Graphs;
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
		NestedTimingTree(const std::wstring& name, NestedTimingTree* parent = nullptr)
			: m_Name(name), m_Parent(parent), m_IsExpanded(false) {}

		NestedTimingTree* GetChild(const std::wstring& name)
		{
			auto iter = m_LUT.find(name);
			if (iter != m_LUT.end())
				return iter->second;

			NestedTimingTree* node = new NestedTimingTree(name, this);
			node->m_Level = m_Level + 1;
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

		inline ScopeTimeData GetScopeData()
		{
			return { m_Name, m_LastStartTick - s_preLastFrameTick, m_LastEndTick - s_preLastFrameTick, (unsigned char)m_Level };
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
			D_ASSERT(!AwaitingUpdate);
			m_StartTick = D_TIME::SystemTime::GetCurrentTick();
			if (Context == nullptr)
				return;

			m_GpuTimer.Start(*Context);

			Context->PIXBeginEvent(m_Name.c_str());
		}

		void StopTiming(CommandContext* Context)
		{
			D_ASSERT(!AwaitingUpdate);
			m_EndTick = D_TIME::SystemTime::GetCurrentTick();
			if (Context == nullptr)
				return;

			m_GpuTimer.Stop(*Context);

			Context->PIXEndEvent();
		}

		void GatherTimes(uint32_t FrameIndex)
		{
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

			m_LastStartTick = m_StartTick;
			m_LastEndTick = m_EndTick;
			m_StartTick = 0;
			m_EndTick = 0;
		}

		void ScopeTimerSnapshot(D_CONTAINERS::DVector<ScopeTimeData>& container)
		{
			container.push_back(GetScopeData());
			for (auto node : m_Children)
				node->ScopeTimerSnapshot(container);
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

		static void PushProfilingMarker(const std::wstring& name, CommandContext* Context);
		static void PopProfilingMarker(CommandContext* Context);
		static void Update(void);
		static void UpdateTimes(void)
		{
			uint32_t FrameIndex = (uint32_t)D_GRAPHICS::GetFrameCount();
			uint64_t frameTick = D_TIME::SystemTime::GetCurrentTick();

			D_PROFILING_GPU::BeginReadBack();
			auto delta =
				sm_RootScope.m_StartTick = s_lastFrameTick;
			sm_RootScope.m_EndTick = frameTick;
			sm_RootScope.GatherTimes(FrameIndex);
			s_FrameDelta.RecordStat(FrameIndex, D_TIME::SystemTime::TimeBetweenTicks(s_lastFrameTick, frameTick));
			D_PROFILING_GPU::EndReadBack();

			float TotalCpuTime, TotalGpuTime;
			sm_RootScope.SumInclusiveTimes(TotalCpuTime, TotalGpuTime);
			s_TotalCpuTime.RecordStat(FrameIndex, TotalCpuTime);
			s_TotalGpuTime.RecordStat(FrameIndex, TotalGpuTime);

			s_preLastFrameTick = s_lastFrameTick;
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

		friend void CpuProfilerValueGetter(float* startTimestamp, float* endTimestamp, unsigned char* level, std::wstring* caption, const void* data, int idx);
		friend void ScopeTimerSnapshot(D_CONTAINERS::DVector<ScopeTimeData>& container);

		void DeleteChildren(void)
		{
			for (auto node : m_Children)
				delete node;
			m_Children.clear();
		}

		std::wstring m_Name;
		NestedTimingTree* m_Parent;
		D_CONTAINERS::DVector<NestedTimingTree*> m_Children;
		D_CONTAINERS::DUnorderedMap<std::wstring, NestedTimingTree*> m_LUT;
		uint64_t m_StartTick;
		uint64_t m_EndTick;
		uint64_t m_LastStartTick;
		uint64_t m_LastEndTick;
		StatHistory m_CpuTime;
		StatHistory m_GpuTime;
		int m_Level = 1;
		bool m_IsExpanded;
		GpuTimer m_GpuTimer;
		static StatHistory s_TotalCpuTime;
		static StatHistory s_TotalGpuTime;
		static StatHistory s_FrameDelta;
		static NestedTimingTree sm_RootScope;
		static NestedTimingTree* sm_CurrentNode;
		static NestedTimingTree* sm_SelectedScope;
		static uint64_t s_lastFrameTick;
		static uint64_t s_preLastFrameTick;
	};

	StatHistory NestedTimingTree::s_TotalCpuTime;
	StatHistory NestedTimingTree::s_TotalGpuTime;
	StatHistory NestedTimingTree::s_FrameDelta;
	NestedTimingTree NestedTimingTree::sm_RootScope(L"");
	NestedTimingTree* NestedTimingTree::sm_CurrentNode = &NestedTimingTree::sm_RootScope;
	NestedTimingTree* NestedTimingTree::sm_SelectedScope = &NestedTimingTree::sm_RootScope;
	uint64_t NestedTimingTree::s_lastFrameTick = D_TIME::SystemTime::GetCurrentTick();
	uint64_t NestedTimingTree::s_preLastFrameTick = s_lastFrameTick;

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
		AwaitingUpdate = false;
		NestedTimingTree::UpdateTimes();
	}

	void FinishFrame()
	{
		AwaitingUpdate = true;
	}

	void ScopeTimerSnapshot(D_CONTAINERS::DVector<ScopeTimeData>& container)
	{
		NestedTimingTree::sm_RootScope.ScopeTimerSnapshot(container);
	}

	void CpuProfilerValueGetter(float* startTimestamp, float* endTimestamp, unsigned char* level, std::wstring* caption, const void* data, int idx)
	{
		auto nestedTree = reinterpret_cast<ScopeTimeData const*>(data)[idx];

		if (startTimestamp)
		{
			auto value = D_TIME::SystemTime::TicksToMillisecs(nestedTree.StartTick);

			*startTimestamp = value >= 0 ? (float)value : 0;
		}

		if (endTimestamp)
		{

			auto value = D_TIME::SystemTime::TicksToMillisecs(nestedTree.EndTick);

			*endTimestamp = value >= 0 ? (float)value : 0;
		}

		if (level)
			*level = (unsigned char)nestedTree.Depth;

		if (caption)
			*caption = nestedTree.Name;
	}

	void BeginBlock(const std::wstring& name, CommandContext* Context)
	{
		if (!AwaitingUpdate)
			NestedTimingTree::PushProfilingMarker(name, Context);
	}

	void EndBlock(CommandContext* Context)
	{
		if (!AwaitingUpdate)
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

	void NestedTimingTree::PushProfilingMarker(const std::wstring& name, CommandContext* Context)
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

	}

}