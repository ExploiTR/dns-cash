#include <utils/tlru_cache.h>

dns_cash::TLRUCache::TLRUCache(uint_fast32_t max_capacity, bool enable_ttl_eviction) :
	max_capacity_(max_capacity), enable_ttl_eviction_(enable_ttl_eviction)
{
	// Apply fixed-point overflow ratio (scaled by 128) to compute hard capacity limit.
	// Right shift by 7 to rescale after multiplication, so - max_capacity_ * 1.2.

	//I dont think this optimization was necessary at all, although done for fun, I'd not use this in a shared codebase.
	//code removed : un-necessary work
	//this->max_capacity_hard_limit_ = ((uint_fast64_t)this->max_capacity_ * this->max_capacity_overflow_ratio_) >> 7;
}

dns_cash::TLRUCache::~TLRUCache()
{
	// just fr structure
}


void dns_cash::TLRUCache::add(const DNSQuestion& question, const DNSAnswer& answer)
{
	//auto unlocked
	std::unique_lock<std::shared_mutex> cache_update_lock(this->cache_map_mutex);

	auto cache_entry = this->query_cache_map.find(question);

	if (cache_entry != this->query_cache_map.end()) {
		cache_entry->second.first = answer;

		// Move its list entry to the back, no copy
		this->query_frequency_list.splice(this->query_frequency_list.end(), this->query_frequency_list, cache_entry->second.second);

		//take the latest, we cant replace answer_old itself because our list iterator holds reference on it.
		//code removed : std::list::splice transfers the node without invalidating pointers or iterators to it
		//cache_entry->second.second = std::prev(query_frequency_list.end());
	}
	else {

		if (this->query_cache_map.size() >= this->max_capacity_) {
			////erase all dnsquestion from last to until we hit max capacity
			////code removed : un-necessary work
			//while (this->query_cache_map.size() > this->max_capacity_) {

			//	//erase from map and list
			//	this->query_cache_map.erase(*oldest_it);
			//	//returns the next valid iterator post removal
			//	oldest_it = this->query_frequency_list.erase(oldest_it);
			//}

			//defensive check
			if (!this->query_frequency_list.empty()) {
				auto oldest_it = this->query_frequency_list.begin();

				this->query_cache_map.erase(*oldest_it);
				this->query_frequency_list.erase(oldest_it);
			}
		}

		//update list
		this->query_frequency_list.push_back(question);  //back is latest
		//update map
		this->query_cache_map.emplace(
			question,
			std::make_pair(answer, std::prev(this->query_frequency_list.end()))
		);
	}
}

void dns_cash::TLRUCache::remove(const DNSQuestion& question)
{
	//auto unlocked
	std::unique_lock<std::shared_mutex> cache_update_lock(this->cache_map_mutex);

	auto cache_entry = this->query_cache_map.find(question);

	if (cache_entry != this->query_cache_map.end()) {
		this->query_frequency_list.erase(cache_entry->second.second);
		this->query_cache_map.erase(cache_entry);
	}
}

bool dns_cash::TLRUCache::has(const DNSQuestion& question)
{
	//auto unlocked -  shared read lock
	std::shared_lock<std::shared_mutex> cache_read_lock(this->cache_map_mutex);

	return this->query_cache_map.contains(question);
}

dns_cash::CacheMap::iterator dns_cash::TLRUCache::find(const DNSQuestion& question)
{
	//auto unlocked -  shared read lock
	std::shared_lock<std::shared_mutex> cache_read_lock(this->cache_map_mutex);

	return this->query_cache_map.find(question);
}

std::optional<DNSAnswer> dns_cash::TLRUCache::get(const DNSQuestion& question)
{
	//auto unlocked -  shared read lock
	std::shared_lock<std::shared_mutex> cache_read_lock(this->cache_map_mutex);

	auto cache_entry = this->query_cache_map.find(question);

	//check existance
	if (cache_entry == this->query_cache_map.end()) return std::nullopt;

	//check ttl enabled and expired
	//here to maintain high thruput read, no removal of the entry will be pushed out eventually
	if (this->enable_ttl_eviction_ && cache_entry->second.first.expiry <= std::chrono::steady_clock::now().time_since_epoch().count())
		return std::nullopt;

	return cache_entry->second.first;
}


