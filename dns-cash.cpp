// dns-cash.cpp : Defines the entry point for the application.
//

#include "dns-cash.h"
#include "callback/DNSRequestCallback.h" 
#include "server/DNSServer.h"     
#include "executors/ThreadPool.h"
#include <random>
#include <thread>
#include <chrono>

int main()
{

	try {
		ThreadPool threads(ThreadPool::get_optimal_thread_count());

		DNSServer server(6073);
		DNSRequestCallback callback(threads,server);

		server.listen(callback);

		return 0;
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return 1;
	}
}