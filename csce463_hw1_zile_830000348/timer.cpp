#include "pch.h"
#include "timer.h"

timer::timer() {
	std::chrono::steady_clock::time_point start_t;
	std::chrono::steady_clock::time_point end_t;
}

void timer::start() {
	start_t = std::chrono::high_resolution_clock::now();
}

int timer::end() {
	end_t = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_t - start_t);
	//std::cout << duration.count() << " ms";
	return duration.count();
}