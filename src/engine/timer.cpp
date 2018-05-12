class Timer
{
	private:
		u64 start_time;
		static u64 perf_count_freq;
		
	public:
		Timer() { if(perf_count_freq == 0) perf_count_freq = SDL_GetPerformanceFrequency(); }
		u64 start() { start_time = getWallClock(); return start_time; }
		u64 getWallClock() { return SDL_GetPerformanceCounter(); }
		f32 getSecondsElapsed() { f32 result = ((f32)(getWallClock() - start_time) / (f32)perf_count_freq); return result; }
		f32 getMillisecondsElapsed() { return getSecondsElapsed() * 1000.0f; }
};

u64 Timer::perf_count_freq = 0;