#pragma once
#include <iostream>
#include "ICallback.h"
#include <chrono>
#include <executors/ThreadPool.h>
#include <server/DNSServer.h>
#include <processor/dns_parser.h>
#include <cinttypes>

class DNSRequestCallback : public ICallback {
private:
	ThreadPool& thread_pool;
	DNSServer& server;

public:
	DNSRequestCallback(ThreadPool& pool, DNSServer& server_) : thread_pool(pool), server(server_) {};

	void onReceive(const char* msg, int len, sockaddr_in& sender) override {
		const char* msgcp = std::move(msg);
		this->thread_pool.enqueue_task(
			[this, msgcp, len, sender]
			{
				using clock = std::chrono::high_resolution_clock;

				auto t0 = clock::now();

				DNSHeader header;
				DNSQuestion question;

				bool res = parse_dns_query(msgcp, len, header, question);

				auto t1 = clock::now();
				auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

				double qps = 1e9 / duration_ns;

				// Correct format for atomic uint64_t
				printf("%lld ns | %s | %.0f req/s\n",
					duration_ns,
					question.qname,
					qps);

				if (!res) return;
			}
		);
	}
};
