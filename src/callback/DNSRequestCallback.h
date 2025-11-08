#pragma once
#include <iostream>
#include "ICallback.h"
#include <chrono>
#include <executors/ThreadPool.h>
#include <server/DNSServer.h>
#include <processor/dns_processor.h>

class DNSRequestCallback : public ICallback {
private:
	ThreadPool& thread_pool;
	DNSServer& server;
public:
	DNSRequestCallback(ThreadPool& pool, DNSServer& server_) : thread_pool(pool), server(server_) {};

	void onReceive(const char* msg, int len, sockaddr_in& sender) override {
		this->thread_pool.enqueue_task(
			[this, msg, len, sender]
			{
				long long sv = duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				DNSHeader header;
				DNSQuestion question;
				parse_query(msg, len, header, question);
				long long ev = duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

				std::cout << question.qname << std::endl;
				std::cout << (ev - sv) << std::endl;
			}
		);
	}
};
