// dns-cash.cpp : Defines the entry point for the application.
//

#include "dns-cash.h"
#include "callback/DNSRequestCallback.h" 
#include "server/DNSServer.h"     
#include "executors/ThreadPool.h"
#include <random>
#include <thread>
#include <chrono>
#include <utils/cmdl_honly.h>
#include <utils/tlru_cache.h>

int main(int argc, char* argv[])
{
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::cout.tie(nullptr);

	try {
		dns_cash::CMDLineParser parser(argc, argv); //todo use?

		ThreadPool threads(ThreadPool::get_optimal_thread_count(2), true);
		dns_cash::TLRUCache tlru(1e6, true); //todo take from cmd command

		DNSServer server(6073);
		DNSRequestCallback callback(threads, server, tlru);

		server.listen(callback);

		return 0;
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return 1;
	}
}