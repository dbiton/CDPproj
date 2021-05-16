#pragma once

#include "CountMinSketch.h"
#include "RedirGraph.h"

#include <vector>
#include <mutex>
#include <stdint.h>

uint32_t hash(uint32_t v, uint64_t key, uint32_t num_buckets);

class RingSketch {
	std::vector<CountMinSketch*> sketchs;
	std::vector<std::mutex*> sketchs_mutexes;
	RedirGraph redir_graph;

	float error_amount;
	uint32_t size_initial, size_curr;
	float total_added;
public:
	RingSketch(float err_amount_initial, int num_sketch_initial = 1);

	void add(uint32_t e);
	float query(uint32_t e);
	void expand();
	void shrink();
	int numSketchs();
private:
	uint32_t getSketchIdx(uint32_t e);

	unsigned getFullestSketchIdx();
	unsigned getEmptiestSketchIdx();
	
	void lockAll();
	void unlockAll();
};