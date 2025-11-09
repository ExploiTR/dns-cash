#include "dns_processor.h"

void process_domain_info(DNSQuestion dns_query, dns_cash::TLRUCache cache_map)
{
	//If our cache still has it, we'll reuse it.
	if (cache_map.has(dns_query)) {
		cache_map.get(dns_query);
	}
	else {

	}
}
