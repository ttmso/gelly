#ifndef NVAPI_ENABLED
#error "NVAPI must be enabled to sample timings"
#endif

// https://developer.nvidia.com/blog/profiling-dxr-shaders-with-timer-instrumentation/
uint timediff(uint startTime, uint endTime)
{
	// Account for (at most one) overflow
	return endTime >= startTime ? (endTime-startTime) : (~0u-(startTime-endTime));
}

#define START_TIMER uint startTime = NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_LO);
#define END_TIMER uint endTime = NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_LO); \
	uint elapsed = timediff(startTime, endTime); \
	float dt = saturate((float)elapsed / 65000.f); // arbitrary value

