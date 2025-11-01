#pragma once
#include <iostream>
#include "ICallback.h"
#include <thread>
#include <chrono>

class DNSRequestCallback : public ICallback {
public:
	void onReceive(const char* msg, int len, sockaddr_in& sender) override {
		for (int i = 0; i < len; ++i)
			printf("%02X ", (unsigned char)msg[i]);
		printf("\n");
	}
};
