#include "ThreadPool.h"
#include <iostream>
#include <Windows.h>

//define
unsigned int ThreadPool::core_count;

//initalize our pool and set stop code to false
ThreadPool::ThreadPool(unsigned int thread_count, bool enable_pinning) :
	stop_code(false),
	active_thread_count(thread_count),
	enable_affinity(enable_pinning) {

	if (thread_count < 1) throw std::exception("Invalid thread count!");
	if (core_count < 0) throw std::exception("Invalid core count, use get_optimal_thread_count to pass thread count to enable pinning.");

	this->initialize_workers(this->active_thread_count);
}

unsigned int ThreadPool::get_optimal_thread_count(unsigned int logical_threads_per_core) {
	const auto& hardware_core_ = std::thread::hardware_concurrency();

	if (logical_threads_per_core < 1) throw std::exception("Invalid LTPC value!");
	if (logical_threads_per_core > hardware_core_) throw std::exception("Invalid LTPC value! Greater than reported core count itself.");

	std::cout << "Calculating thread count assuming :" << "\n";
	std::cout << "LTPC :" << logical_threads_per_core << "\n";
	std::cout << "Multiplier :" << thread_multiplier << "\n\n";

	core_count = hardware_core_ / logical_threads_per_core;

	//6 logical core = 6 * opt(4) = 24 | but if ltpc = 2 in the system core count will be reported 12 then 4/2 = 2*12=24
	return (hardware_core_ * ThreadPool::thread_multiplier) / logical_threads_per_core;
}

void ThreadPool::initialize_workers(unsigned int thread_count) {
	if (thread_count < 1) throw std::exception("Invalid thread count!");
	std::cout << "Initializing with worker count - " << thread_count << "\n";
	std::cout << "CPU Core Pinning - " << (this->enable_affinity ? "Enabled" : "Disabled") << "\n";

	for (unsigned int core_index = 0; core_index < thread_count; core_index++) {
		if (this->enable_affinity) {
			std::thread c_thread = this->get_worker_thread();

			// Get the native handle of the thread (windows)
			HANDLE hThread = c_thread.native_handle();

			//core mask
			int core_mask = core_index % this->core_count;  // 43 % 6 = 1
			DWORD_PTR affinity_mask = (1ULL << core_mask);

			// Set the thread affinity mask
			DWORD_PTR oldMask = SetThreadAffinityMask(hThread, affinity_mask);

			if (oldMask == 0) {
				std::cerr << "Failed to set thread affinity. Error: " << GetLastError() << " \n Disable pinning if required." << std::endl;
				return;
			}

			//Construction build error : push_back requires a copy constructor if you pass an lvalue (c_thread), 
			//and since std::thread can only be moved, the STL tries internal construction (std::construct_at) and fails.
			this->workers.push_back(std::move(c_thread));
		}
		else {
			// although its one time activity we avoid an extra move
			this->workers.emplace_back(this->get_worker_thread());
		}
	}
}

std::thread ThreadPool::get_worker_thread() {
	return std::thread([this]
		{
			//loop until asked to stop
			while (!this->stop_code) {
				//lock guard cant auto unlock so we need a RAII lock that supports manual unlock/relock
				std::unique_lock<std::mutex> lock(this->qutex);

				//this thread will now wait until tasks are available or stop code is triggered.
				//Atomic: unlock, sleep, reacquire lock, check predicate. Prevents false wakeups.
				this->cvar.wait(lock, [this] { return !this->tasks.empty() || this->stop_code; });

				//incase here for stop code.
				if (!this->tasks.empty()) {
					//get the task and remove from queue
					const std::function<void()> task = std::move(this->tasks.front()); //actual copy with move semantics
					this->tasks.pop();

					//we can unlock now before executing
					lock.unlock();

					try {
						//running the actual task.
						task();
					}
					catch (std::exception& ex) {
						std::cerr << "Error in worker thread - " << ex.what() << std::endl;
					}
				}

				// Lock auto-unlocks here when it goes out of scope (if still locked)
			}
		});
}

void ThreadPool::enqueue_task(const std::function<void()>& task) {
	{
		//obtain lock update queue and notify any single thread.
		std::unique_lock<std::mutex> lock(qutex);
		tasks.push(task);
	}
	cvar.notify_one();
}

void ThreadPool::stop_workers() {
	{
		//we dont want the threads to lock the queue again when they wake up
		std::unique_lock<std::mutex> lock(qutex);
		stop_code = true;
	}

	//thats one thundering herd
	cvar.notify_all();

	for (auto& worker : workers)
		worker.join();
}