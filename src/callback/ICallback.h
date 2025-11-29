#pragma once
#include <WinSock2.h>
#include <functional>

class ICallback {
public:
	virtual void onReceive(const char* msg, int len, sockaddr_in& sender) = 0;
	virtual ~ICallback() = default;
};
