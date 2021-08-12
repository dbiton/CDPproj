#pragma once

#include "CountMinSketch.h"
#include "RedirGraph.h"

#include <map>
#include <set>
#include <mutex>


class RingSketch {
	std::map<uint32_t, CountMinSketch*> sketchs;
	std::map<uint32_t, std::mutex*> sketchs_mutexes;
	std::mutex mutex_expand_shrink;
	RedirGraph redir_graph;

	float error_amount;
	uint32_t size_initial, size_curr, total_added;
public:
	RingSketch(float err_amount_initial, int num_sketch_initial, int num_hh);

	void add(uint32_t e);
	void add(std::set<uint32_t> es);
	int query(uint32_t e);
	int query(const std::set<uint32_t>& es);
	void expand();
	void shrink();
	int numSketchs();
private:
	uint32_t hashSet(const std::set<uint32_t>& es);
	uint32_t getSketchIdx(uint32_t e);

	uint32_t getFullestSketchIdx();
	uint32_t getEmptiestSketchIdx(uint32_t& second_emptiest);
	void lockAll();
	void unlockAll();
};