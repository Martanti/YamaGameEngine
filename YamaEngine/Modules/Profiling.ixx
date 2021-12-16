#ifdef PROFILING
// CPU And memory retrieval is based on
// https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process [2021/11/28]
module;
#include <Windows.h>;
#include "psapi.h"

export module YaMaProfiling;
import <chrono>;

namespace yaprfl
{
	// Memory

	PROCESS_MEMORY_COUNTERS_EX pmc;
	MEMORYSTATUSEX memInfo;

	// CPU

	static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	static int numProcessors;
	static HANDLE self;

	/**
	 * @brief Initialize the data needed for profiling.
	*/
	export void Init()
	{
		// CPU
		SYSTEM_INFO sysInfo;
		FILETIME ftime, fsys, fuser;

		GetSystemInfo(&sysInfo);
		numProcessors = sysInfo.dwNumberOfProcessors;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&lastCPU, &ftime, sizeof(FILETIME));

		self = GetCurrentProcess();
		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
		memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
	}

	/**
	 * @brief Get used RAM by the process.
	*/
	export size_t GetUsedMemory()
	{
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		return pmc.WorkingSetSize;
	}

	/**
	 * @brief Get the process usage of CPU in %.
	*/
	export double GetCPUUsage()
	{
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, user;
		double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (sys.QuadPart - lastSysCPU.QuadPart) +
			(user.QuadPart - lastUserCPU.QuadPart);
		percent /= (now.QuadPart - lastCPU.QuadPart);
		percent /= numProcessors;
		lastCPU = now;
		lastUserCPU = user;
		lastSysCPU = sys;

		return percent * 100;
	}

	/**
	 * @brief Helper class for calculating passed time.
	*/
	export class StopWatch
	{
	private:
		/**
		 * @brief Internal clock.
		*/
		std::chrono::time_point<std::chrono::steady_clock> mStartTime;
	public:

		/**
		 * @brief Set the internal clock.
		*/
		inline void Start()
		{
			mStartTime = std::chrono::steady_clock::now();
		}

		/**
		 * @brief Get the time passed since the internal clock was set.
		 * @return
		*/
		inline float SecondsPassed()
		{
			auto currentTime = std::chrono::steady_clock::now();
			std::chrono::duration<float> elapsedTime = currentTime - mStartTime;
			return elapsedTime.count();
		}
	};
}

#endif // PROFILING
