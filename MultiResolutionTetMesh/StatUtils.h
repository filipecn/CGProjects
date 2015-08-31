#include <chrono>
#include <ctime>
#include <iostream>

class Timer {
	std::chrono::time_point<std::chrono::system_clock> tick, tack;

	public:

	void start(){
		tick = std::chrono::system_clock::now();
	}

	double check(){
		tack = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = tack-tick;
		//std::time_t end_time = std::chrono::system_clock::to_time_t(end);
		return elapsed_seconds.count();
	}

	void report(){
		std::cerr << check() << std::endl;	
	}
};
