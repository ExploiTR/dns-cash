#include <utils/tlru_cache.h>

dns_cash::TLRUCache::TLRUCache(uint_fast32_t max_capacity, uint_fast8_t max_capacity_overflow_ratio, bool enable_ttl_eviction) :
	max_capacity_(max_capacity), max_capacity_overflow_ratio_(max_capacity_overflow_ratio), enable_ttl_eviction_(enable_ttl_eviction)
{
	// Apply fixed-point overflow ratio (scaled by 128) to compute hard capacity limit.
	// Right shift by 7 to rescale after multiplication, so - max_capacity_ * 1.2.

	//I dont think this optimization was necessary at all, although done for fun, I'd not use this in a shared codebase.
	this->max_capacity_hard_limit_ = ((uint_fast64_t)this->max_capacity_ * this->max_capacity_overflow_ratio_) >> 7;
}

void dns_cash::TLRUCache::add(const DNSQuestion& question)
{
	if (this->current_size_ > this->max_capacity_hard_limit_) {
		//perform cleanup
	}


}

void dns_cash::TLRUCache::remove(const DNSQuestion& question)
{
}

bool dns_cash::TLRUCache::has(const DNSQuestion& question)
{
	return false;
}

void dns_cash::TLRUCache::get(const DNSQuestion& question)
{
}


