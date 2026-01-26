#pragma once
#include <string>
#include "utils/tlru_cache.h"
#include "dns_parser.h"

void process_domain_info(const DNSQuestion& dns_query, dns_cash::TLRUCache cache_map);