#pragma once
#include <iostream>
#include "ICallback.h"
#include <chrono>
#include <executors/ThreadPool.h>
#include <server/DNSServer.h>
#include <processor/dns_parser.h>
#include <cinttypes>
#include <utils/tlru_cache.h>

class DNSRequestCallback final : public ICallback
{
private:
    ThreadPool& thread_pool;
    DNSServer& server;
    dns_cash::TLRUCache& dns_cache;

    sockaddr_in dns_resolver_addr;
    using clock = std::chrono::steady_clock;

public:
    DNSRequestCallback(ThreadPool& pool, DNSServer& server_, dns_cash::TLRUCache& cache) : thread_pool(pool),
        server(server_), dns_cache(cache)
    {
        dns_resolver_addr.sin_family = AF_INET;
        dns_resolver_addr.sin_port = htons(server.get_port());
        //converts the value from the host's to network short byte order (big endian)

        inet_pton(AF_INET, "8.8.8.8", &dns_resolver_addr.sin_addr);
        //Converts IP string into a binary format for sockaddr_in.
    };

    void onReceive(const char* msg, int len, sockaddr_in& sender) override
    {
        //immediate copy needed.
        std::vector<char> packet_data(msg, msg + len);

        this->thread_pool.enqueue_task(
            [this, dns_msg = std::move(packet_data), &len, &sender]
            {
                const auto t0 = clock::now();

                ///////////// parsing

                DNSHeader header;
                DNSQuestion question;

                //todo : need to parse header and qsn separately - because header cna be for answer as well (recursive resolver)
                const uint_fast16_t answer_offset = parse_dns_query(dns_msg.data(), len, header, question);
                if (!answer_offset) return;

                if (header.qr)
                {
                    //in our case, this is a real dns resolver replying us
                    printf("%d - %s - %d", question.qclass, question.qname, question.qtype);
                    ///dns_cache.add(question,);
                    //todo cache this response and send to client.
                    //todo little parsing required, need to store the ttl I think
                }
                else
                {
                    //this is a client making a query

                    if (auto answer = this->dns_cache.get(question))
                    {
                        //this->server.send_to_client((const char*)answer->rdata, 512);
                    }
                    else
                    {
                        //we're done here, in case we receive the response in some other thread or cycle.
                        this->server.send_to_server(dns_msg.data(), dns_msg.size());
                        printf("srv rq st\n");
                    }
                }

                ///////////// parsing

                auto t1 = clock::now();
                auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

                double qps = 1e9 / duration_ns;

                char ip_str[INET_ADDRSTRLEN];

                inet_ntop(AF_INET, &sender.sin_addr, ip_str, sizeof(ip_str));

                // Correct format for atomic uint64_t
                printf("%lld ns | %s | q: %d | s %s | %.0f req/s\n",
                       duration_ns,
                       question.qname,
                       header.qr,
                       ip_str,
                       qps);

                if (!answer_offset) return;
            }
        );
    }
};
