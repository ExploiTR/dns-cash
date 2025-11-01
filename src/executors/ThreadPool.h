#pragma once
#include <vector>
#include <thread>
#include <exception>

class ThreadPool {
private:
	int nthreads = 1;
	int thread_multiplier = 4;
	std::vector<std::thread> thread_list;

public:
	//ltpc = Logical Thread Per Core, should be >=1
	unsigned int get_optimal_thread_count(int ltpc = 1) const;
	void initialize_threads(int count = 1);
};