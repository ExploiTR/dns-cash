#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <WinSock2.h>
#include "callback/ICallback.h"

class DNSServer {
private:
	unsigned short dnsPort = 53;
	SOCKET socket_ = INVALID_SOCKET;
	bool runsock_ = false;
	bool start_internal();

	const char* art = R"(
 /$$$$$$$  /$$   /$$  /$$$$$$           /$$$$$$   /$$$$$$   /$$$$$$  /$$   /$$
| $$__  $$| $$$ | $$ /$$__  $$         /$$__  $$ /$$__  $$ /$$__  $$| $$  | $$
| $$  \ $$| $$$$| $$| $$               | $$     | $$  \ $$| $$      | $$  | $$
| $$  | $$| $$ $$ $$|  $$$$$$  /$$$$$$ | $$     | $$$$$$$$|  $$$$$$ | $$$$$$$$
| $$  | $$| $$  $$$$ \____  $$|______/ | $$     | $$__  $$ \____  $$| $$__  $$
| $$  | $$| $$\  $$$ /$$  \ $$         | $$     | $$  | $$ /$$  \ $$| $$  | $$
| $$$$$$$/| $$ \  $$|  $$$$$$/         |  $$$$$$| $$  | $$|  $$$$$$/| $$  | $$
|_______/ |__/  \__/ \______/           \______/|__/  |__/ \______/ |__/  |__/
------------------------------------------------------------------------------
Running ~)";

public:
	DNSServer() = default;
	~DNSServer() { stop(); }
	bool start(unsigned short port = 53);
	void listen(ICallback& callback) const;
	bool stop();
	void send();
};