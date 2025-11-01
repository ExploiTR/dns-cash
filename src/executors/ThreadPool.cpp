#include "ThreadPool.h"

unsigned int ThreadPool::get_optimal_thread_count(int ltpc) const {
	return std::thread::hardware_concurrency() * (this->thread_multiplier / ltpc); //6 logical core = 24 - but if ltpc =2 it'll be reported 12 then 4/2 =2*12=24
}

void ThreadPool::initialize_threads(int count) {
	if (count < 1) throw std::exception("Invalid thread count!");

	this->nthreads = count;
	for (int _ = 0; _ < count; _++)
		this->thread_list.push_back(std::thread());
}