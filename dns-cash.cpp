// dns-cash.cpp : Defines the entry point for the application.
//

#include "dns-cash.h"
#include "callback/DNSRequestCallback.h" 
#include "server/DNSServer.h"     
#include "executors/ThreadPool.h"

int main()
{
	try {
		ThreadPool threads;
		threads.initialize_threads(threads.get_optimal_thread_count());

		DNSServer server;
		DNSRequestCallback callback;
		server.start(6073);
		server.listen(callback);
		return 0;
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return 1;
	}
}