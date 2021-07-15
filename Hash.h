#pragma once

#include <stdint.h>

uint64_t hash(const uint32_t& v, const uint32_t& k, const uint32_t& mod) {
	uint64_t n = (uint64_t)v << 32 | k;
	uint64_t p = 0x5555555555555555ULL;		// pattern of alternating 0 and 1
	uint64_t c = 0x6666DEADBABE3333;	// random uneven integer constant;
	
	auto xorshift = [](const uint64_t& n, int i) {
		return n ^ (n >> i);
	};
	
	return c * xorshift(p * xorshift(n, 32), 32) % mod;
}