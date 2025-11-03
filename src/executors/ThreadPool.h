#pragma once
#include <vector>
#include <thread>
#include <exception>
#include <concurrent_queue.h>
#include <queue>
#include <mutex>
#include <functional>

class ThreadPool {
private:
	static const int thread_multiplier = 4;

	int active_thread_count = 1;

	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex qutex; //sounds cute-x -> queue mutex xd
	std::condition_variable cvar; //our beloved war horn
	std::atomic<bool> stop_code{ false };

	std::thread get_worker_thread();

	ThreadPool();

public:
	ThreadPool(int thread_count);
	~ThreadPool() { stop_workers(); };

	//ltpc = Logical Thread Per Core, should be >=1
	unsigned static int get_optimal_thread_count(int logical_threads_per_core = 1);
	void initialize_workers(int thread_count = 1);
	void stop_workers();

	void enqueue_task(const std::function<void()>& task);
	
};