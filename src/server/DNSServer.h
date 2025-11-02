#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <WinSock2.h>
#include "callback/ICallback.h"

class DNSServer {
private:
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

	unsigned short dns_port = 53;
	SOCKET socket_ = INVALID_SOCKET;
	bool runsock_ = false;

	DNSServer();
	bool start_internal();
	bool start(unsigned short port = 53);
	void listen(ICallback& callback) const;

public:
	DNSServer(unsigned short port, ICallback& callback);
	~DNSServer() { stop(); }
	bool stop();
	void send();
};