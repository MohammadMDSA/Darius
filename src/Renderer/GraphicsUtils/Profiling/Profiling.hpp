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

#pragma once

#include "Renderer/CommandContext.hpp"

#include <string>

#ifndef D_PROFILING
#define D_PROFILING Darius::Graphics::Utils::Profiling
#endif

using namespace D_GRAPHICS;

namespace Darius::Graphics::Utils::Profiling
{
	class StatHistory;

	void Update();

	void BeginBlock(const std::wstring& name, CommandContext* Context = nullptr);
	void EndBlock(CommandContext* Context = nullptr);

	void Pause();
	void Resume();
	bool IsPaused();

	float GetAvgCpuTime();
	float GetAvgGpuTime();
	float GetAvgFrameDelta();
	float GetLastCpuTime();
	float GetLastGpuTime();
	float GetLastFrameDelta();
	float GetMinCpuTime();
	float GetMinGpuTime();
	float GetMinFrameDelta();
	float GetMaxCpuTime();
	float GetMaxGpuTime();
	float GetMaxFrameDelta();


#ifdef RELEASE
	class ScopedTimer
	{
	public:
		ScopedTimer(const std::wstring&) {}
		ScopedTimer(const std::wstring&, CommandContext&) {}
	};
#else
	class ScopedTimer
	{
	public:
		ScopedTimer(const std::wstring& name) : m_Context(nullptr)
		{
			D_PROFILING::BeginBlock(name);
		}
		ScopedTimer(const std::wstring& name, CommandContext& Context) : m_Context(&Context)
		{
			D_PROFILING::BeginBlock(name, m_Context);
		}
		~ScopedTimer()
		{
			D_PROFILING::EndBlock(m_Context);
		}

	private:
		CommandContext* m_Context;
	};
#endif

	class StatHistory
	{
	public:
		StatHistory()
		{
			for (uint32_t i = 0; i < kHistorySize; ++i)
				m_RecentHistory[i] = 0.0f;
			for (uint32_t i = 0; i < kExtendedHistorySize; ++i)
				m_ExtendedHistory[i] = 0.0f;
			m_Average = 0.0f;
			m_Minimum = 0.0f;
			m_Maximum = 0.0f;
		}

		void RecordStat(uint32_t FrameIndex, float Value)
		{
			m_RecentHistory[FrameIndex % kHistorySize] = Value;
			m_ExtendedHistory[FrameIndex % kExtendedHistorySize] = Value;
			m_Recent = Value;

			uint32_t ValidCount = 0;
			m_Minimum = FLT_MAX;
			m_Maximum = 0.0f;
			m_Average = 0.0f;

			for (float val : m_RecentHistory)
			{
				if (val > 0.0f)
				{
					++ValidCount;
					m_Average += val;
					m_Minimum = XMMin(val, m_Minimum);
					m_Maximum = XMMax(val, m_Maximum);
				}
			}

			if (ValidCount > 0)
				m_Average /= (float)ValidCount;
			else
				m_Minimum = 0.0f;
		}

		float GetLast(void) const { return m_Recent; }
		float GetMax(void) const { return m_Maximum; }
		float GetMin(void) const { return m_Minimum; }
		float GetAvg(void) const { return m_Average; }

		const float* GetHistory(void) const { return m_ExtendedHistory; }
		uint32_t GetHistoryLength(void) const { return kExtendedHistorySize; }

	private:
		static const uint32_t kHistorySize = 64;
		static const uint32_t kExtendedHistorySize = 256;
		float m_RecentHistory[kHistorySize];
		float m_ExtendedHistory[kExtendedHistorySize];
		float m_Recent;
		float m_Average;
		float m_Minimum;
		float m_Maximum;
	};

}