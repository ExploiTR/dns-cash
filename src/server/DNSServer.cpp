// DNSServer.cpp
#pragma once
//https://github.com/computersarecool/cpp_sockets/blob/master/src/simple_cpp_sockets.h

#include "DNSServer.h"
#include <exception>
#include <iostream>
#include <ws2tcpip.h>
#include <mutex>

DNSServer::DNSServer(unsigned short port) : dns_port(port) {
	this->start(dns_port);
}

bool DNSServer::start(unsigned short port) {
	try {
		//im not sure why i decided to init WSA calls in another fucntion but it just looks good.
		if (!start_internal()) throw std::exception("WSA Error");

		//initialize udp socket
		socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (socket_ == INVALID_SOCKET) {
			std::cerr << "socket() failed: " << WSAGetLastError() << "\n";
			WSACleanup();
			return false;
		}

		sockaddr_in current_addr{};
		current_addr.sin_family = AF_INET;
		current_addr.sin_port = htons(this->dns_port); //converts the value from the host's to network short byte order (big endian)

		inet_pton(AF_INET, "0.0.0.0", &current_addr.sin_addr); //Converts IP string into a binary format for sockaddr_in.

		if (bind(socket_, reinterpret_cast<sockaddr*>(&current_addr), sizeof(current_addr)) == SOCKET_ERROR) {
			std::cerr << "bind() failed on port (" << this->dns_port << ") - " << WSAGetLastError() << std::endl;
			closesocket(socket_);
			WSACleanup();
			return false;
		}

		runsock_ = true;
		return runsock_;
	}
	catch (std::exception& e) {
		std::cout << __FILE__ << "(" << __LINE__ << ")" << " - DNS Server error at init : " << e.what() << std::endl;
		return false;
	}
}

void DNSServer::listen(ICallback& callback) {
	if (runsock_) {
		this->listen_ = true;
		std::cout << DNSServer::art << " on -> port : " << this->dns_port << std::endl;
		sockaddr_in from_addr;
		int from_addr_len = sizeof(from_addr);

		char buffer[512]; // RFC 1035

		while (runsock_) {
			int recvLen = recvfrom(socket_, buffer, 512, 0, (SOCKADDR*)&from_addr, &from_addr_len);
			if (recvLen) callback.onReceive(buffer, recvLen, from_addr);
		}
	}
	else throw std::system_error(213, std::generic_category(), std::string("DNS Server Is Not Initialized"));
}

void DNSServer::send(sockaddr_in& to_addr, const char* resp, int resp_len) {
	std::lock_guard<std::mutex> lock(this->send_mutex);
	int ret = sendto(this->socket_, resp, resp_len, 0, (SOCKADDR*)&to_addr, sizeof(to_addr));
	if (ret < 0)
		throw std::system_error(WSAGetLastError(), std::system_category(), "Error : DNS Server sendto failed!");
}

//init windows socket api/wsa 
bool DNSServer::start_internal() {
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr << "WSAStartup failed: " << iResult << std::endl;
		return false;
	}
	return true;
}

bool DNSServer::stop() {
	try {
		if (socket_ != INVALID_SOCKET) closesocket(socket_);
		this->listen_ = false;
		this->runsock_ = false;
		WSACleanup();
		return true;
	}
	catch (std::exception& e) {
		std::cerr << "DNS Server failed to stop : " << e.what() << std::endl;
		return false;
	}
}



