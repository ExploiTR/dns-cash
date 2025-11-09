#pragma once
#include <concurrent_unordered_map.h>
#include <processor/dns_parser.h>


namespace dns_cash {

	/*
		LRU Cache implementation which respects TTL Input (optionally).
	*/
	class TLRUCache {
	private:
		TLRUCache();

		uint_fast32_t currentSize = 0;
		uint_fast32_t currentCapacity = 0;
		uint_fast32_t maxCapacity = 0;

		uint16_t dns_query_id;

		//mapping id to a Request or Response.
		concurrency::concurrent_unordered_map<uint16_t, DNSRequest> dns_cache_map;

	public:
		TLRUCache(uint_fast32_t maxCapacity = 1e6, bool enable_ttl_eviction = true);

		void add(uint16_t dns_query_id);
		void remove(uint16_t dns_query_id);
		bool has(uint16_t dns_query_id);
	};
}
