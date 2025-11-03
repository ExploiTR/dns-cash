#include "ThreadPool.h"
#include <iostream>

//initalize our pool and set stop code to false
ThreadPool::ThreadPool(int thread_count) : stop_code(false), active_thread_count(thread_count) {
	if (thread_count < 1) throw std::exception("Invalid thread count!");

	this->initialize_workers(this->active_thread_count);
}

unsigned int ThreadPool::get_optimal_thread_count(int logical_threads_per_core) {
	if (logical_threads_per_core < 1) throw std::exception("Invalid LTPC value!");

	//6 logical core = 24 - but if ltpc = 2 it'll be reported 12 then 4/2 =2*12=24
	return (std::thread::hardware_concurrency() * ThreadPool::thread_multiplier) / logical_threads_per_core;
}

void ThreadPool::initialize_workers(int thread_count) {
	if (thread_count < 1) throw std::exception("Invalid thread count!");
	std::cout << "Initializing with worker count - " << thread_count << std::endl;

	for (int _ = 0; _ < thread_count; _++) {
		// although its one time activity we avoid an extra move
		this->workers.emplace_back(this->get_worker_thread());
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
						std::cout << "Error in worker thread - " << ex.what() << std::endl;
					}
				}

				// Lock auto-unlocks here when it goes out of scope (if still locked)
			}
		});
}

void ThreadPool::enqueue_task(const std::function<void()>& task) {
	{
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