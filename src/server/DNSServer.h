#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <WinSock2.h>
#include "callback/ICallback.h"
#include <mutex>
#include <ws2tcpip.h>

class DNSServer {
private:
	const char* art = R"(
------------------------------------------------------------------------------
 /$$$$$$$  /$$   /$$  /$$$$$$            /$$$$$$  /$$$$$$   /$$$$$$  /$$   /$$
| $$__  $$| $$$ | $$ /$$__  $$          /$$__  $$/$$__  $$ /$$__  $$| $$  | $$
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
	bool listen_ = false;

	std::mutex send_mutex;

	DNSServer();
	bool start_internal();
	bool start(unsigned short port = 53);

public:
	DNSServer(unsigned short port);
	~DNSServer() { stop(); }

	void send(sockaddr_in& to_addr, const char* resp, int resp_len);
	void listen(ICallback& callback);
	bool stop();
	unsigned short get_port() { return dns_port; };
};