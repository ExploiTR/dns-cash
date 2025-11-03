#pragma once
#include <iostream>
#include "ICallback.h"
#include <chrono>
#include <executors/ThreadPool.h>
#include <server/DNSServer.h>

class DNSRequestCallback : public ICallback {
private:
	ThreadPool& thread_pool;
	DNSServer& server;
public:
	DNSRequestCallback(ThreadPool& pool, DNSServer& server_) : thread_pool(pool), server(server_) {};

	void onReceive(const char* msg, int len, sockaddr_in& sender) override {

		//this->thread_pool.enqueue_task([this, msg, len, sender] {
		//	//todo
		//	});
	}
};
