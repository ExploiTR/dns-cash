// DNSServer.cpp
#pragma once
//https://github.com/computersarecool/cpp_sockets/blob/master/src/simple_cpp_sockets.h

#include "DNSServer.h"
#include <exception>
#include <iostream>

DNSServer::DNSServer(unsigned short port) : dns_port(port)
{
    this->start(dns_port);
}

bool DNSServer::start(unsigned short port)
{
    try
    {
        //im not sure why I decided to init WSA calls in another function, but it just looks good.
        if (!start_internal()) throw std::exception("WSA Error");

        //create dns server addr --> for sending
        this->sock_remote_addr.sin_family = AF_INET;
        this->sock_remote_addr.sin_port = htons(this->dns_remote_port);
        inet_pton(AF_INET, this->remote_dns_address, &sock_remote_addr.sin_addr);

        //initialize udp socket
        socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        //initialize udp socket
        if (socket_ == INVALID_SOCKET)
        {
            std::cerr << "socket_ --> socket() failed: " << WSAGetLastError() << "\n";
            WSACleanup();
            return false;
        }

        sockaddr_in sock_server_addr{};
        sock_server_addr.sin_family = AF_INET;
        sock_server_addr.sin_port = htons(this->dns_port);
        //converts the value from the host's to network short byte order (big endian)

        inet_pton(AF_INET, "0.0.0.0", &sock_server_addr.sin_addr);
        //Converts IP string into a binary format for sockaddr_in.

        if (bind(socket_, reinterpret_cast<sockaddr*>(&sock_server_addr), sizeof(sock_server_addr)) == SOCKET_ERROR)
        {
            std::cerr << "bind() failed on port (" << this->dns_port << ") - " << WSAGetLastError() << std::endl;
            closesocket(socket_);
            WSACleanup();
            return false;
        }

        runsock_ = true;
        return runsock_;
    }
    catch (std::exception& e)
    {
        std::cout << __FILE__ << "(" << __LINE__ << ")" << " - DNS Server error at init : " << e.what() << std::endl;
        return false;
    }
}

void DNSServer::listen(ICallback& callback)
{
    if (!this->runsock_) throw std::system_error(213, std::generic_category(), "DNS Server Is Not Initialized");

    this->listen_ = true;
    std::cout << DNSServer::art << " on -> port : " << this->dns_port << std::endl;
    sockaddr_in from_addr{};
    int from_addr_len = sizeof(from_addr);

    char buffer[4096]; // RFC 1035 is 512, but I don't want to drop packets.

    while (this->runsock_)
    {
        if (const int recvLen = recvfrom(socket_, buffer, 4096, 0, reinterpret_cast<SOCKADDR*>(&from_addr), &from_addr_len); recvLen > 0)
            callback.onReceive(buffer, recvLen, from_addr);
    }
}

void DNSServer::send_to_server(const char* resp, int resp_len)
{
    // sendto is thread-safe for UDP.
    int ret = sendto(this->socket_, resp, resp_len, 0, reinterpret_cast<SOCKADDR*>(&this->sock_remote_addr),
                     sizeof(this->sock_remote_addr));

    if (ret < 0)
    {
        // UDP can drop. If a packet drops, the client will retry.
        // Need to log the error and move on.

        const int err = WSAGetLastError();

        printf("exx --> %d || %d \n", ret, err);
        if (err != WSAEWOULDBLOCK && err != WSAEINTR)
        {
            //need 2 log
        }
    }
}

void DNSServer::send_to_client(const sockaddr_in& client_addr, const char* resp, const int resp_len)
{
    SOCKET client_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // sendto is thread-safe for UDP.
    int ret = sendto(client_sock, resp, resp_len, 0, (SOCKADDR*)&client_addr, sizeof(client_addr));

    if (ret < 0)
    {
        // UDP can drop. If a packet drops, the client will retry.
        // Need to log the error and move on.

        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && err != WSAEINTR)
        {
            //need 2 log
        }
    }
}


//init windows socket api/wsa
bool DNSServer::start_internal()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return false;
    }
    return true;
}

bool DNSServer::stop()
{
    try
    {
        if (socket_ != INVALID_SOCKET) closesocket(socket_);
        this->listen_ = false;
        this->runsock_ = false;
        WSACleanup();
        return true;
    }
    catch (std::exception& e)
    {
        std::cerr << "DNS Server failed to stop : " << e.what() << std::endl;
        return false;
    }
}
