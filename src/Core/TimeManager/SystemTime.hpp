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
// Contains classes needed to time executing code.
//


#ifndef D_TIME
#define D_TIME Darius::Core::TimeManager
#endif // !D_TIME

namespace Darius::Core::TimeManager
{

    class SystemTime
    {
    public:

        // Query the performance counter frequency
        static void Initialize(void);

        // Query the current value of the performance counter
        static int64_t GetCurrentTick(void);

        static void BusyLoopSleep(float SleepTime);

        static inline double TicksToSeconds(int64_t TickCount)
        {
            return TickCount * sm_CpuTickDelta;
        }

        static inline double TicksToMillisecs(int64_t TickCount)
        {
            return TickCount * sm_CpuTickDelta * 1000.0;
        }

        static inline double TimeBetweenTicks(int64_t tick1, int64_t tick2)
        {
            return TicksToSeconds(tick2 - tick1);
        }

    private:

        // The amount of time that elapses between ticks of the performance counter
        static double sm_CpuTickDelta;
    };

}
