#pragma once
#include <chrono>
class timer
{
public:
	timer();
	void start();
	int end();
private:
	std::chrono::steady_clock::time_point start_t;
	std::chrono::steady_clock::time_point end_t;
};

