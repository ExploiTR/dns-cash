#pragma once
#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <processor/dns_parser.h>
#include <optional>

namespace dns_cash {

	using ListIt = std::list<DNSQuestion>::iterator;
	using CacheEntry = std::pair<DNSAnswer, ListIt>;
	using CacheMap = std::unordered_map<DNSQuestion, CacheEntry>;

	/*
		LRU Cache implementation which respects TTL Input (optionally).
	*/
	class TLRUCache {
	private:
		TLRUCache();

		uint_fast32_t max_capacity_ = 0;

		//Batch eviction was a bad idea - unused now.
		uint_fast64_t max_capacity_hard_limit_ = -1;

		// Fixed-point scale (×128) representing 1.2 multiplier for 20% overflow allowance.
		// Used to allow capacity to grow up to 120% before trimming back to max (prevents rapid oscillation at full load).
		// premature unnesessary optimizations lol
		//code removed : un-necessary work
		//uint_fast16_t max_capacity_overflow_ratio_ = (uint_fast16_t)(1.2 * 128);

		bool enable_ttl_eviction_ = true;

		//mapping id to a Request or Response.
		std::unordered_map<DNSQuestion, CacheEntry> query_cache_map;
		std::list<DNSQuestion> query_frequency_list;

		//map mutex; this is read-heavy once the query is cached.
		std::shared_mutex cache_map_mutex;

	public:
		TLRUCache(uint_fast32_t max_capacity, bool enable_ttl_eviction);
		~TLRUCache();

		void add(const DNSQuestion& question, const DNSAnswer& answer);
		void remove(const DNSQuestion& question);
		bool has(const DNSQuestion& question);

		CacheMap::iterator find(const DNSQuestion& question);

		//DNS Answer
		std::optional<DNSAnswer> get(const DNSQuestion& question);
	};
}
