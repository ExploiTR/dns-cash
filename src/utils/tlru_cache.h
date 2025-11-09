#pragma once
#include <unordered_map>
#include <list>
#include <processor/dns_parser.h>

namespace dns_cash {

	/*
		LRU Cache implementation which respects TTL Input (optionally).
	*/
	class TLRUCache {
	private:
		TLRUCache();

		uint_fast32_t current_size_ = 0;
		uint_fast32_t current_capacity_ = 0;
		uint_fast32_t max_capacity_ = 0;
		uint_fast64_t max_capacity_hard_limit_ = -1;

		// Fixed-point scale (×128) representing 1.2 multiplier for 20% overflow allowance.
		// Used to allow capacity to grow up to 120% before trimming back to max (prevents rapid oscillation at full load).
		uint_fast16_t max_capacity_overflow_ratio_ = (uint16_t)(1.2 * 128);
		bool enable_ttl_eviction_ = true;

		//mapping id to a Request or Response.
		std::unordered_map<DNSQuestion, int> query_cache_map;
		std::list<DNSQuestion> query_frequency_list;

	public:
		TLRUCache(uint_fast32_t max_capacity, uint_fast8_t max_capacity_overflow_ratio, bool enable_ttl_eviction);

		void add(const DNSQuestion& question);
		void remove(const DNSQuestion& question);
		bool has(const DNSQuestion& question);

		//todo change DNS Answer
		void get(const DNSQuestion& question);
	};
}
