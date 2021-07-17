#pragma once

#include <map>
#include "RingSketch.h"

struct TesterSettings {
	unsigned iter_num, thread_num, key_min, key_max, ring_size_min, ring_size_max, ring_size_init, ring_add_per_modify_size, num_heavy_hitters;
	float error_margin;
};

struct TesterResults {
	float error_ring;
	float error_rglr;
};

class Tester
{
	RingSketch* sketch_ring;
	CountMinSketch* sketch_regular;
	std::map<int, int>* hashtable;
	TesterSettings ts;
	TesterResults tr;
	std::mutex mutex;
public:
	TesterResults test(const TesterSettings& ts);
private:
	void run_thread();
};

